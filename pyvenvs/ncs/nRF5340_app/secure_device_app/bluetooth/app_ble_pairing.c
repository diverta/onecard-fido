/* 
 * File:   app_ble_pairing.c
 * Author: makmorit
 *
 * Created on 2021/04/27, 10:18
 */
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_ble_pairing);

static void pairing_confirm(struct bt_conn *conn)
{
    bt_conn_auth_pairing_confirm(conn);
}

static void pairing_complete(struct bt_conn *conn, bool bonded)
{
    (void)conn;
    LOG_INF("Pairing completed %s", bonded ? "(bonding done)" : "");
}

static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
    (void)conn;
    LOG_ERR("Pairing failed (reason=%d)", reason);
}

static void pairing_cancel(struct bt_conn *conn)
{
    (void)conn;
    LOG_INF("Pairing canceled");
}

static const struct bt_conn_auth_cb conn_auth_callbacks = {
    .pairing_confirm = pairing_confirm,
    .pairing_complete = pairing_complete,
    .pairing_failed = pairing_failed,
    .cancel = pairing_cancel,
};

bool app_ble_pairing_init(void)
{
    // ペアリング時のコールバックを設定
    int rc = bt_conn_auth_cb_register(&conn_auth_callbacks);
    if (rc != 0) {
        LOG_ERR("bt_conn_auth_cb_register returns %d", rc);
        return false;
    }

    return true;
}
