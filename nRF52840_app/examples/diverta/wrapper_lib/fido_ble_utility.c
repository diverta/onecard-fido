/* 
 * File:   fido_ble_utility.c
 * Author: makmorit
 *
 * Created on 2022/12/19, 16:54
 */
#include "sdk_common.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_ble_utility
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// プラットフォーム依存モジュール
#include "fido_ble_pairing.h"

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

void fido_ble_unpairing_on_disconnect(void)
{
    if (m_peer_id_set) {
        // 接続の切断検知時点で、
        // peer_id に対応するペアリング情報を削除
        m_peer_id_set = false;
        NRF_LOG_DEBUG("Unpairing process for peer_id=0x%04x done.", m_peer_id_to_unpair);
    }
}
