/* 
 * File:   fido_ble_unpairing.c
 * Author: makmorit
 *
 * Created on 2022/12/31, 11:20
 */
#include <zephyr/types.h>
#include <zephyr/kernel.h>

#include "app_ble_define.h"
#include "app_ble_unpairing.h"
#include "app_event.h"

// プラットフォーム非依存モジュール
#include "fido_common.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(fido_ble_unpairing);

// ペアリング解除対象の peer_id を保持
static uint16_t m_peer_id_to_unpair = PEER_ID_NOT_EXIST;

bool fido_ble_unpairing_request(uint8_t *request_buffer, size_t request_size, uint8_t *response_buffer, size_t *response_size)
{
    if (request_size == 0) {
        // データが無い場合（peer_id 取得要求の場合）
        // ペアリング済みデバイスを走査し、peer_idを取得
        if (app_ble_unpairing_get_peer_id(&m_peer_id_to_unpair) == false) {
            return false;
        }

        // peer_id をレスポンス領域に設定
        fido_set_uint16_bytes(response_buffer, m_peer_id_to_unpair);
        *response_size = 2;
        return true;

    } else if (request_size == 2) {
        // データにpeer_idが指定されている場合
        // 接続の切断検知時点で、
        // peer_id に対応するペアリング情報を削除
        m_peer_id_to_unpair = fido_get_uint16_from_bytes(request_buffer);
        LOG_INF("Unpairing will process for peer_id=0x%04x", m_peer_id_to_unpair);

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
    LOG_INF("Unpairing process for peer_id=0x%04x canceled.", m_peer_id_to_unpair);

    // peer_idを初期化
    m_peer_id_to_unpair = PEER_ID_NOT_EXIST;
}

void fido_ble_unpairing_on_disconnect(void)
{
    if (m_peer_id_to_unpair != PEER_ID_NOT_EXIST) {
        // ペアリング解除対象の peer_id をクリア
        uint16_t peer_id_to_unpair = m_peer_id_to_unpair;
        m_peer_id_to_unpair = PEER_ID_NOT_EXIST;

        // 接続の切断検知時点で、peer_id に対応するペアリング情報を削除
        if (app_ble_unpairing_delete_peer_id(peer_id_to_unpair) == false) {
            LOG_ERR("Unpairing process for peer_id=0x%04x failed", peer_id_to_unpair);
            return;
        }

        // ペアリング解除要求が成功時は、スリープ状態に遷移
        printk("Unpairing process for peer_id=0x%04x done \n", peer_id_to_unpair);
        app_event_notify(APEVT_IDLING_DETECTED);
    }
}

void fido_ble_unpairing_done(bool success, uint16_t peer_id)
{
    // nRF5340アプリケーションでは実装不要
}

//
// ペアリング情報の全削除処理
//
bool fido_ble_unpairing_erase_bond_data(void (*_response_func)(bool))
{
    return app_ble_unpairing_delete_all_peers(_response_func);
}

bool fido_ble_unpairing_erase_bond_data_completed(void const *evt)
{
    // nRF52840アプリケーションに用意した関数のため、
    // nRF5340では何もしません
    //   app_ble_pairing_erase_bond_data 内で
    //   等価の処理が行われます
    return false;
}
