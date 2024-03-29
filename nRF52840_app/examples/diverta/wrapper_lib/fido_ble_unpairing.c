/* 
 * File:   fido_ble_unpairing.c
 * Author: makmorit
 *
 * Created on 2022/12/19, 16:54
 */
#include "sdk_common.h"
#include "peer_manager.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_ble_unpairing
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// プラットフォーム依存モジュール
#include "fido_ble_pairing.h"
#include "fido_board.h"

// プラットフォーム非依存モジュール
#include "fido_common.h"

// ペアリング解除対象があるかどうかを保持
static bool m_peer_id_set;

// ペアリング解除対象の peer_id を保持
static uint16_t m_peer_id_to_unpair;

bool fido_ble_unpairing_request(uint8_t *request_buffer, size_t request_size, uint8_t *response_buffer, size_t *response_size)
{
    m_peer_id_set = false;
    
    if (request_size == 0) {
        // データが無い場合（peer_id 取得要求の場合）
        // 現在接続中のデバイスに対応する peer_id を取得
        uint16_t peer_id;
        if (fido_ble_pairing_get_peer_id(&peer_id) == false) {
            return false;
        }

        // peer_id をレスポンス領域に設定
        fido_set_uint16_bytes(response_buffer, peer_id);
        *response_size = 2;
        return true;

    } else if (request_size == 2) {
        // データにpeer_idが指定されている場合
        // 接続の切断検知時点で、
        // peer_id に対応するペアリング情報を削除
        m_peer_id_to_unpair = fido_get_uint16_from_bytes(request_buffer);
        m_peer_id_set = true;
        NRF_LOG_DEBUG("Unpairing will process for peer_id=0x%04x", m_peer_id_to_unpair);

        // レスポンスは無し
        *response_size = 0;
        return true;

    } else {
        return false;
    }
}

void fido_ble_unpairing_cancel_request(void)
{
    // ペアリング情報削除の実行を回避
    m_peer_id_set = false;
    NRF_LOG_DEBUG("Unpairing process for peer_id=0x%04x canceled.", m_peer_id_to_unpair);
}

void fido_ble_unpairing_on_disconnect(void)
{
    if (m_peer_id_set) {
        // 接続の切断検知時点で、
        // peer_id に対応するペアリング情報を削除
        m_peer_id_set = false;
        fido_ble_pairing_delete_peer_id(m_peer_id_set);
        NRF_LOG_DEBUG("Unpairing process for peer_id=0x%04x processing...", m_peer_id_to_unpair);
    }
}

void fido_ble_unpairing_done(bool success, uint16_t peer_id)
{
    if (success) {
        // ペアリング解除要求が成功時は、スリープ状態に遷移
        NRF_LOG_INFO("Unpairing process for peer_id=0x%04x done.", peer_id);
        fido_board_prepare_for_deep_sleep();

    } else {
        NRF_LOG_ERROR("Unpairing process for peer_id=0x%04x failed.", peer_id);
    }
}
//
// ペアリング情報の全削除処理
//
static void (*erase_bonding_data_response_func)(bool) = NULL;

bool fido_ble_unpairing_erase_bond_data(void (*_response_func)(bool))
{
    // ペアリング情報削除後に実行される関数の参照を退避
    erase_bonding_data_response_func = _response_func;

    // 全てのペアリング情報を削除
    ret_code_t err_code = pm_peers_delete();
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("pm_peers_delete returns 0x%02x ", err_code);
        return false;
    }

    return true;
}

static void perform_erase_bond_data_response_func(bool success)
{
    if (erase_bonding_data_response_func == NULL) {
        return;
    }

    // ペアリング情報削除後に実行される処理
    (*erase_bonding_data_response_func)(success);
    erase_bonding_data_response_func = NULL;
}

bool fido_ble_unpairing_erase_bond_data_completed(void const *evt)
{
    pm_evt_t const *p_evt = (pm_evt_t const *)evt;
    if (p_evt->evt_id == PM_EVT_PEERS_DELETE_SUCCEEDED) {
        NRF_LOG_DEBUG("pm_peers_delete has completed successfully");
        perform_erase_bond_data_response_func(true);
        return true;
    }

    if (p_evt->evt_id == PM_EVT_PEERS_DELETE_FAILED) {
        NRF_LOG_ERROR("pm_peers_delete has failed");
        perform_erase_bond_data_response_func(false);
        return true;
    }

    return false;
}
