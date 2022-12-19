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

bool fido_ble_unpairing_request(uint8_t *request_buffer, size_t request_size, uint8_t *response_buffer, size_t *response_size)
{
    // 元データチェック
    if (request_size > 1) {
        return false;
    }

    if (request_size == 0) {
        // 現在接続中のデバイスに対応する peer_id を取得
        uint16_t peer_id;
        if (fido_ble_pairing_get_peer_id(&peer_id) == false) {
            return false;
        }

        // peer_id をレスポンス領域に設定
        fido_set_uint16_bytes(response_buffer, peer_id);
        *response_size = 2;

    } else {
        // TODO: 仮の実装です。
        NRF_LOG_DEBUG("Command unpairing request 0x%02x", request_buffer[0]);
        response_size = 0;
    }

    return true;
}
