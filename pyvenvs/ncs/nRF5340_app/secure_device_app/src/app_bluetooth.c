/* 
 * File:   app_bluetooth.c
 * Author: makmorit
 *
 * Created on 2021/04/06, 14:50
 */
#include <zephyr.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>

//
// for Bluetooth smp service
//
#include <mgmt/mcumgr/smp_bt.h>
#include "os_mgmt/os_mgmt.h"
#include "img_mgmt/img_mgmt.h"

// for BLE pairing
#include "app_ble_pairing.h"
#include "app_event.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_bluetooth);

//
// アドバタイズ関連
//
// work queue for advertise
static struct k_work advertise_work;
static struct k_work stop_advertise_work;

// advertising data
static struct bt_data ad[3];
static struct bt_data ad_nobredr = BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR);
static struct bt_data ad_limited = BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_LIMITED | BT_LE_AD_NO_BREDR));
static struct bt_data ad_uuid_smp = BT_DATA_BYTES(BT_DATA_UUID128_ALL, 0x84, 0xaa, 0x60, 0x74, 0x52, 0x8a, 0x8b, 0x86, 0xd3, 0x4c, 0xb7, 0x1d, 0x1d, 0xdc, 0x53, 0x8d);
static struct bt_data ad_uuid_16 = BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(0xfffd), BT_UUID_16_ENCODE(BT_UUID_DIS_VAL));

//
// BLEアドバタイズ開始
//
static void advertise(struct k_work *work)
{
    // ペアリングモードに応じ、
    // アドバタイズデータ（flags）を変更
    (void)work;
    if (app_ble_pairing_mode()) {
        ad[0] = ad_limited;
    } else {
        ad[0] = ad_nobredr;
    }

    // サービスUUIDを設定
    ad[1] = ad_uuid_smp;
    ad[2] = ad_uuid_16;

    // アドバタイジングを開始する
    bt_le_adv_stop();
    int rc = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
    if (rc) {
        LOG_ERR("Advertising failed to start (rc %d)", rc);
        return;
    }

    LOG_INF("Advertising successfully started (%s mode)", app_ble_pairing_mode() ? "Pairing" : "Non-Pairing");

    // BLEアドバタイズ開始イベントを業務処理スレッドに引き渡す
    app_event_notify(APEVT_BLE_ADVERTISE_STARTED);
}

static bool k_work_submission(struct k_work *p_work, const char *p_name)
{
    int rc = k_work_submit_to_queue(&k_sys_work_q, p_work);
    if (rc == -EBUSY) {
        LOG_ERR("%s submission failed (work item is cancelling / work queue is draining or plugged)", p_name);
        return false;
        
    } else if (rc == -EINVAL) {
        LOG_ERR("%s submission failed (work queue is null and the work item has never been run)", p_name);
        return false;
        
    } else {
        return true;
    }
}

bool app_ble_start_advertising(void)
{
    return k_work_submission(&advertise_work, "Advertising");
}

//
// BLEアドバタイズ停止
//
static void advertise_stop(struct k_work *work)
{
    // アドバタイジングを停止
    (void)work;
    int rc = bt_le_adv_stop();
    LOG_INF("Advertising stopped (rc=%d)", rc);

    // BLEアドバタイズ停止イベントを業務処理スレッドに引き渡す
    app_event_notify(APEVT_BLE_ADVERTISE_STOPPED);
}

bool app_ble_stop_advertising(void)
{
    return k_work_submission(&stop_advertise_work, "Stop advertise");
}

static void connected(struct bt_conn *conn, uint8_t err)
{
    (void)conn;
    if (err) {
        LOG_ERR("Connection failed (err 0x%02x)", err);
    } else {
        LOG_INF("Connected");

        // BLE接続イベントを業務処理スレッドに引き渡す
        app_event_notify(APEVT_BLE_CONNECTED);
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    (void)conn;
    LOG_INF("Disconnected (reason 0x%02x)", reason);

    // BLE切断イベントを業務処理スレッドに引き渡す
    app_event_notify(APEVT_BLE_DISCONNECTED);
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
    app_ble_start_advertising();
}

void app_bluetooth_start(void)
{
    // BLE SMPサービスの設定
    os_mgmt_register_group();
    img_mgmt_register_group();

    // ペアリングモードを設定
    if (app_ble_pairing_mode_set(false) == false) {
        LOG_ERR("Pairing mode set failed");
        return;
    }

    // アドバタイズ処理を work queue に入れる
    k_work_init(&advertise_work, advertise);
    k_work_init(&stop_advertise_work, advertise_stop);

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
