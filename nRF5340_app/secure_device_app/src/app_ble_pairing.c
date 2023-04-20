/* 
 * File:   app_ble_pairing.c
 * Author: makmorit
 *
 * Created on 2021/04/27, 10:18
 */
#include <zephyr/types.h>
#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_ble_pairing);

#define LOG_BONDED_PEER_ADDRESS false

// ペアリング済みの数を保持
static uint8_t m_bonded_count = 0;

static void count_bonded(const struct bt_bond_info *info, void *data)
{
    (void)data;
    m_bonded_count++;

#if LOG_BONDED_PEER_ADDRESS
    uint8_t adr[BT_ADDR_SIZE];
    for (int i = 0; i < BT_ADDR_SIZE; i++) {
        adr[BT_ADDR_SIZE - i - 1] = info->addr.a.val[i];
    }
    LOG_HEXDUMP_DBG(adr, BT_ADDR_SIZE, "Bonded peer address");
#else
    (void)info;
#endif
}

uint8_t app_ble_pairing_get_peer_count(void)
{
    m_bonded_count = 0;
    bt_foreach_bond(BT_ID_DEFAULT, count_bonded, NULL);

#if LOG_BONDED_PEER_ADDRESS
    LOG_DBG("Bonded peer count=%u", m_bonded_count);
#endif

    return m_bonded_count;
}

// ペアリングモードを保持
static bool m_pairing_mode = false;

static void pairing_confirm(struct bt_conn *conn)
{
    if (m_pairing_mode == false) {
        // ペアリングモードでない場合は、
        // ペアリング要求に応じないようにする
        int rc = bt_conn_auth_cancel(conn);
        if (rc != 0) {
            LOG_ERR("bt_conn_auth_cancel returns %d", rc);
        } else {
            LOG_DBG("Pairing refused");
        }
    }
}

static void bond_deleted(uint8_t id, const bt_addr_le_t *addr)
{
    (void)id;
    char addr_str[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
    LOG_INF("Bonding information deleted: address=%s", addr_str);
}

static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
    char addr[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
    LOG_INF("Passkey for %s: %06u", addr, passkey);
}

static void auth_cancel(struct bt_conn *conn)
{
    if (m_pairing_mode) {
        char addr[BT_ADDR_LE_STR_LEN];
        bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
        LOG_WRN("Pairing with authentication cancelled: %s", addr);

    } else {
        LOG_INF("Pairing canceled");
    }
}

static void auth_pairing_complete(struct bt_conn *conn, bool bonded)
{
    (void)conn;
    LOG_INF("Pairing with authentication completed %s", bonded ? "(bonded)" : "(not bonded)");
}

static void auth_pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
    (void)conn;
    if (m_pairing_mode) {
        LOG_ERR("Pairing with authentication failed (reason=%d)", reason);

    } else {
        if (reason == BT_SECURITY_ERR_AUTH_REQUIREMENT) {
            LOG_ERR("Pairing failed (The requested security level could not be reached)");
        } else {
            LOG_ERR("Pairing failed (reason=%d)", reason);
        }
    }
}

static const struct bt_conn_auth_cb cb_for_pair = {
    .pairing_confirm = pairing_confirm,
    .passkey_display = auth_passkey_display,
    .passkey_entry = NULL,
    .cancel = auth_cancel,
};

struct bt_conn_auth_info_cb info_cb_for_pair = {
    .pairing_complete = auth_pairing_complete,
    .pairing_failed = auth_pairing_failed,
    .bond_deleted = bond_deleted,
};

bool app_ble_pairing_register_callbacks(void)
{
    // コールバックを設定
    int rc = bt_conn_auth_cb_register(&cb_for_pair);
    if (rc != 0) {
        LOG_ERR("bt_conn_auth_cb_register returns %d", rc);
        return false;
    }
    rc = bt_conn_auth_info_cb_register(&info_cb_for_pair);
    if (rc != 0) {
        LOG_ERR("bt_conn_auth_info_cb_register returns %d", rc);
        return false;
    }
    return true;
}

bool app_ble_pairing_mode_set(bool b)
{
    // ペアリングモードを設定
    m_pairing_mode = b;
    return true;
}

bool app_ble_pairing_mode(void)
{
    return m_pairing_mode;
}

void app_ble_pairing_mode_initialize(void)
{
    // ペアリング情報の登録件数を照会
    bool run_as_pairing_mode = false;
    uint8_t peer_count = app_ble_pairing_get_peer_count();
    if (peer_count == 0) {
        // ペアリング情報が存在しない場合は、優先してペアリングモードとする
        LOG_INF("Already bonded peer is not exist.");
        run_as_pairing_mode = true;

    } else {
        // ペアリング情報が１件以上存在すれば、非ペアリングモードとする
        LOG_INF("Already bonded peer is exist (count=%d).", peer_count);
    }

    // ペアリングモードを設定
    if (app_ble_pairing_mode_set(run_as_pairing_mode) == false) {
        LOG_ERR("Initial pairing mode set failed");
    }
}
