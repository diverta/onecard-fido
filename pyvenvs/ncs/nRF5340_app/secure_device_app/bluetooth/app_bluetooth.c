/* 
 * File:   app_bluetooth.c
 * Author: makmorit
 *
 * Created on 2021/04/06, 14:50
 */
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>

//
// for Bluetooth smp service
//
#include <mgmt/mcumgr/smp_bt.h>
#include "os_mgmt/os_mgmt.h"
#include "img_mgmt/img_mgmt.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_bluetooth);

// work queue for advertise
static struct k_work advertise_work;

// advertising data
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_LIMITED | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, 0x84, 0xaa, 0x60, 0x74, 0x52, 0x8a, 0x8b, 0x86, 0xd3, 0x4c, 0xb7, 0x1d, 0x1d, 0xdc, 0x53, 0x8d),
};

static void advertise(struct k_work *work)
{
    // アドバタイジングを開始する
    bt_le_adv_stop();
    int rc = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
    if (rc) {
        LOG_ERR("Advertising failed to start (rc %d)", rc);
        return;
    }

    LOG_INF("Advertising successfully started");
}

static void connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        LOG_ERR("Connection failed (err 0x%02x)", err);
    } else {
        LOG_INF("Connected");
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    // アドバタイジング再開
    LOG_INF("Disconnected (reason 0x%02x)", reason);
    k_work_submit(&advertise_work);
}

static struct bt_conn_cb conn_callbacks = {
    .connected = connected,
    .disconnected = disconnected,
};

static void bt_ready(int err)
{
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)", err);
        return;
    }

    // アドバタイジング開始
    LOG_INF("Bluetooth initialized");
    k_work_submit(&advertise_work);
}

void app_bluetooth_start(void)
{
    // BLE SMPサービスの設定
    os_mgmt_register_group();
    img_mgmt_register_group();

    // アドバタイズ処理を work queue に入れる
    k_work_init(&advertise_work, advertise);

    // Enable Bluetooth.
    int rc = bt_enable(bt_ready);
    if (rc != 0) {
        LOG_ERR("Bluetooth init failed (err %d)", rc);
        return;
    }

    // 接続時コールバックの設定
    bt_conn_cb_register(&conn_callbacks);

    // Initialize the Bluetooth mcumgr transport.
    smp_bt_register();
}
