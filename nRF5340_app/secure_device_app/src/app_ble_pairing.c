/* 
 * File:   app_ble_pairing.c
 * Author: makmorit
 *
 * Created on 2021/04/27, 10:18
 */
#include <zephyr/types.h>
#include <zephyr.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>

#include "app_ble_pairing.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_ble_pairing);

// ペアリングモードを保持
static bool m_pairing_mode = false;

static void pairing_confirm(struct bt_conn *conn)
{
    // ペアリングモードでない場合は、
    // ペアリング要求に応じないようにする
    int rc = bt_conn_auth_cancel(conn);
    if (rc != 0) {
        LOG_ERR("bt_conn_auth_cancel returns %d", rc);
    } else {
        LOG_DBG("Pairing refused");
    }
}

static void pairing_complete(struct bt_conn *conn, bool bonded)
{
    (void)conn;
    LOG_INF("Pairing completed %s", bonded ? "(bonded)" : "(not bonded)");
}

static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
    (void)conn;
    if (reason == BT_SECURITY_ERR_AUTH_REQUIREMENT) {
        LOG_ERR("Pairing failed (The requested security level could not be reached)");
        return;
    }
    LOG_ERR("Pairing failed (reason=%d)", reason);
}

static void pairing_cancel(struct bt_conn *conn)
{
    (void)conn;
    LOG_INF("Pairing canceled");
}

static void bond_deleted(uint8_t id, const bt_addr_le_t *addr)
{
    (void)id;
    const uint8_t *data = addr->a.val;
    LOG_INF("Bonding information deleted: address=%x%x%x%x%x%x",
            data[5], data[4], data[3], data[2], data[1], data[0]);
}

static const struct bt_conn_auth_cb cb_for_non_pair = {
    .pairing_confirm = pairing_confirm,
    .pairing_failed = pairing_failed,
    .cancel = pairing_cancel,
    .bond_deleted = bond_deleted,
};

static const struct bt_conn_auth_cb cb_for_pair = {
    .pairing_complete = pairing_complete,
    .pairing_failed = pairing_failed,
    .bond_deleted = bond_deleted,
};

bool register_callbacks(void)
{
    // コールバック設定を解除
    int rc = bt_conn_auth_cb_register(NULL);

    if (m_pairing_mode) {
        // ペアリングモード時のコールバックを設定
        rc = bt_conn_auth_cb_register(&cb_for_pair);
        if (rc != 0) {
            LOG_ERR("bt_conn_auth_cb_register returns %d", rc);
            return false;
        }

    } else {
        // 非ペアリングモード時のコールバックを設定
        rc = bt_conn_auth_cb_register(&cb_for_non_pair);
        if (rc != 0) {
            LOG_ERR("bt_conn_auth_cb_register returns %d", rc);
            return false;
        }
    }

    return true;
}

bool app_ble_pairing_mode_set(bool b)
{
    // ペアリングモードを設定
    m_pairing_mode = b;

    // ペアリングモードに応じて
    // コールバックを再設定
    return register_callbacks();
}

bool app_ble_pairing_mode(void)
{
    return m_pairing_mode;
}

//
// ペアリング情報削除処理
//
bool app_ble_pairing_erase_bond_data(void (*response_func)(bool))
{
    // ボンディングされている全てのペアリング鍵を削除
    int rc = bt_unpair(BT_ID_DEFAULT, BT_ADDR_LE_ANY);
    if (rc != 0) {
        LOG_ERR("bt_unpair returns %d", rc);
        return false;
    }

    // ペアリング情報削除後に実行される処理
    if (response_func != NULL) {
        (*response_func)(true);
    }
    return true;
}
