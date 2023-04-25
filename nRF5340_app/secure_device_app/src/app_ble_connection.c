/* 
 * File:   app_ble_connection.c
 * Author: makmorit
 *
 * Created on 2023/04/24, 16:58
 */
#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>

// for BLE pairing
#include "app_ble_advertise.h"
#include "app_ble_pairing.h"
#include "app_event.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_ble_connection);

//
// 接続関連
//
static bt_addr_le_t *secure_connected_addr = NULL;

void *app_ble_connection_address_get(void)
{
    return (void *)secure_connected_addr;
}

static void connected(struct bt_conn *conn, uint8_t err)
{
    (void)conn;
    if (err) {
        LOG_ERR("Connection failed (err 0x%02x)", err);

    } else {
        LOG_INF("Connected");
        int ret = bt_conn_set_security(conn, BT_SECURITY_L4);
        if (ret != 0) {
            LOG_ERR("Failed to set security (bt_conn_set_security returns %d)", ret);
        }
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    (void)conn;
    LOG_INF("Disconnected (reason 0x%02x)", reason);

    // BLE切断イベントを業務処理スレッドに引き渡す
    app_event_notify(APEVT_BLE_DISCONNECTED);
}

static void identity_resolved(struct bt_conn *conn, const bt_addr_le_t *rpa, const bt_addr_le_t *identity)
{
    char addr_identity[BT_ADDR_LE_STR_LEN];
    char addr_rpa[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(identity, addr_identity, sizeof(addr_identity));
    bt_addr_le_to_str(rpa, addr_rpa, sizeof(addr_rpa));
    LOG_INF("Identity resolved %s -> %s", addr_rpa, addr_identity);
}

static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
    char addr[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
    secure_connected_addr = NULL;

    if (err == BT_SECURITY_ERR_SUCCESS) {
        if (level < BT_SECURITY_L2) {
            LOG_WRN("Security change failed: %s level %u", addr, level);

        } else {
            // セキュリティーレベル変更が成功したら、
            // BLE接続イベントを業務処理スレッドに引き渡す
            LOG_INF("Connected %s with security level %u", addr, level);
            app_event_notify(APEVT_BLE_CONNECTED);

            // 接続先のアドレス情報を保持
            secure_connected_addr = (bt_addr_le_t *)bt_conn_get_dst(conn);
        }

    } else if (err == BT_SECURITY_ERR_PIN_OR_KEY_MISSING) {
        // ペアリング情報の消失を検知（このデバイスにペアリング情報が存在しない）
        LOG_ERR("Pairing information is not exist in this device.");
        app_event_notify(APEVT_BLE_CONNECTION_FAILED);

    } else {
        LOG_WRN("Security failed: %s level %u err %d", addr, level, err);
    }
}

// 接続時コールバックの設定
BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
#if defined(CONFIG_BT_SMP)
    .identity_resolved = identity_resolved,
    .security_changed = security_changed,
#endif
};
