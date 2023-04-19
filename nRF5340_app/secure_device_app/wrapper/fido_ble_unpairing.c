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

// ペアリング解除対象の peer_id を保持
static uint16_t m_peer_id_to_unpair;

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

// 作業領域
static uint8_t work_buf[BT_ADDR_SIZE];
static uint8_t m_bonded_count = 0;

static void match_bonded(const struct bt_bond_info *info, void *data)
{
    // ペアリング済みデバイスのBluetoothアドレスを取得し、
    // 接続中のアドレスと等しいかチェック
    (void)data;
    convert_to_be_address(work_buf, (uint8_t *)info->addr.a.val);
    if (memcmp(work_buf, connected_addr, BT_ADDR_SIZE) == 0) {
        // 等しければ peer_id を設定
        m_peer_id_to_unpair = m_bonded_count;
    }

    // デバイス数をカウントアップ
    m_bonded_count++;
}

static bool get_bonded_peer_id(void)
{
    // peer_idを初期化
    m_peer_id_to_unpair = 0xffff;
    m_bonded_count = 0;

    // ペアリング済みデバイスのBluetoothアドレスを走査
    bt_foreach_bond(BT_ID_DEFAULT, match_bonded, NULL);

    // 接続中デバイスが、ペアリング済みデバイスでない場合
    if (m_peer_id_to_unpair == 0xffff) {
        return false;
    }

#if LOG_CONNECTED_PEER_ADDRESS
    LOG_DBG("Unpairing device found (peer_id=0x%04x)", m_peer_id_to_unpair);
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

        // ペアリング済みデバイスを走査し、peer_idを取得
        if (get_bonded_peer_id() == false) {
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
    m_peer_id_to_unpair = 0xffff;
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
