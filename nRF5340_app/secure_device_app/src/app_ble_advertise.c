/* 
 * File:   app_ble_advertise.c
 * Author: makmorit
 *
 * Created on 2023/04/24, 11:05
 */
#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/settings/settings.h>

#include "app_ble_pairing.h"
#include "app_event.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_ble_advertise);

//
// アドバタイズ関連
//
// work queue for advertise
static struct k_work advertise_work;

// advertising data
static struct bt_data ad[3];
static struct bt_data ad_nobredr = BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR);
static struct bt_data ad_limited = BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_LIMITED | BT_LE_AD_NO_BREDR));

// UUID for FIDO BLE service (0xfffd)
static struct bt_data ad_uuid_fido = BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(0xfffd), BT_UUID_16_ENCODE(BT_UUID_DIS_VAL));

// UUID for BLE SMP service
static struct bt_data ad_uuid_smp = BT_DATA_BYTES(BT_DATA_UUID128_ALL, 0x84, 0xaa, 0x60, 0x74, 0x52, 0x8a, 0x8b, 0x86, 0xd3, 0x4c, 0xb7, 0x1d, 0x1d, 0xdc, 0x53, 0x8d);

// Service data field for FIDO BLE service (0xfffd)
static struct bt_data ad_svcdata = BT_DATA_BYTES(BT_DATA_SVC_DATA16, BT_UUID_16_ENCODE(0xfffd), 0x80);

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

    // BLE FIDOサービスUUIDを設定
    ad[1] = ad_uuid_fido;

    // BLE SMPサービスUUIDを追加設定（非ペアリングモード時）
    size_t ad_len = 2;
    if (app_ble_pairing_mode() == false) {
        ad[ad_len] = ad_uuid_smp;
        ad_len++;
    }

    // サービスデータフィールドを追加設定（ペアリングモード時のみ）
    if (app_ble_pairing_mode()) {
        ad[ad_len] = ad_svcdata;
        ad_len++;
    }

    // アドバタイジングを開始する
    bt_le_adv_stop();
    int rc = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ad_len, NULL, 0);
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

void app_ble_advertise_start(void)
{
    k_work_submission(&advertise_work, "Starting advertise");
}

//
// BLEアドバタイズ停止
//
void app_ble_advertise_stop(void)
{
}

//
// BLEアドバタイズ関連の初期処理
//
void app_ble_advertise_init(void)
{
    // アドバタイズ処理を work queue に入れる
    k_work_init(&advertise_work, advertise);
}
