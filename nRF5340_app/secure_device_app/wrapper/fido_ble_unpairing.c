/* 
 * File:   fido_ble_unpairing.c
 * Author: makmorit
 *
 * Created on 2022/12/31, 11:20
 */
#include <zephyr/types.h>
#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>

#include "app_ble_pairing.h"
#include "app_bluetooth.h"

// プラットフォーム非依存モジュール
#include "fido_common.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(fido_ble_unpairing);

#define LOG_CONNECTED_PEER_ADDRESS  false

// 接続中のアドレスを保持
static uint8_t connected_addr[BT_ADDR_SIZE];

static void convert_to_be_address(uint8_t adr[], uint8_t val[])
{
    for (int i = 0; i < BT_ADDR_SIZE; i++) {
        adr[BT_ADDR_SIZE - i - 1] = val[i];
    }
}

static bool get_connected_peer_address(void)
{
    // 現在接続中のデバイスのBluetoothアドレスを取得
    bt_addr_le_t *addr = (bt_addr_le_t *)app_bluetooth_secure_connected_addr();
    if (addr == NULL) {
        return false;
    }
    // 取得したアドレスを保持
    convert_to_be_address(connected_addr, addr->a.val);

#if LOG_CONNECTED_PEER_ADDRESS
    LOG_HEXDUMP_DBG(connected_addr, BT_ADDR_SIZE, "Connected peer address");
#endif
    return true;
}

bool fido_ble_unpairing_request(uint8_t *request_buffer, size_t request_size, uint8_t *response_buffer, size_t *response_size)
{
    if (request_size == 0) {
        // データが無い場合（peer_id 取得要求の場合）
        // 現在接続中デバイスのBluetoothアドレスを取得
        if (get_connected_peer_address() == false) {
            return false;
        }

        // TODO: 仮の実装です。
        return false;

    } else {
        return false;
    }
}

void fido_ble_unpairing_cancel_request(void)
{
    // TODO: 仮の実装です。
}

void fido_ble_unpairing_on_disconnect(void)
{
    // TODO: 仮の実装です。
}

void fido_ble_unpairing_done(bool success, uint16_t peer_id)
{
    // TODO: 仮の実装です。
}

//
// ペアリング情報の全削除処理
//
bool fido_ble_unpairing_erase_bond_data(void (*_response_func)(bool))
{
    return app_ble_pairing_erase_bond_data(_response_func);
}

bool fido_ble_unpairing_erase_bond_data_completed(void const *evt)
{
    // nRF52840アプリケーションに用意した関数のため、
    // nRF5340では何もしません
    //   app_ble_pairing_erase_bond_data 内で
    //   等価の処理が行われます
    return false;
}
