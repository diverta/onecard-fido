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
#include <settings/settings.h>

// for BLE pairing
#include "app_ble_pairing.h"
#include "app_event.h"
#include "app_ble_fido.h"
#include "app_ble_smp.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_bluetooth);

// Work for BT address string
static char addr_str_buf_1[BT_ADDR_LE_STR_LEN];
static char addr_str_buf_2[BT_ADDR_LE_STR_LEN];

//
// パスキー関連
//
#include <drivers/hwinfo.h>

// Work for hardware ID & passkey
static uint8_t  m_hwid[8];
static uint32_t m_passkey;

static void set_passkey_for_pairing(void)
{
    // BLEペアリング用のパスキーを設定
    uint8_t *p = (uint8_t *)&m_passkey;

    if (hwinfo_get_device_id(m_hwid, sizeof(m_hwid)) > 0) {
        // ハードウェアIDの下位４バイト分を抽出
        for (int i = 0; i < 4; i++) {
            p[3 - i] = m_hwid[i + 4];
        }
        
        // 抽出されたハードウェアID（１０進）の下６桁をパスキーに設定
        m_passkey %= 1000000;

    } else {
        // ハードウェアIDが抽出できなかった場合は、
        // パスキーを'000000'に設定
        m_passkey = 0;
    }
    
    LOG_INF("Passkey for BLE pairing: %06u", m_passkey);
    bt_passkey_set((unsigned int)m_passkey);
}

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
    app_ble_fido_ad_uuid_set(&ad[1]);

    // BLE SMPサービスUUIDを追加設定
    size_t ad_len = 2;
    if (app_ble_smp_ad_uuid_set(&ad[2])) {
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
    (void)conn;

    bt_addr_le_to_str(identity, addr_str_buf_1, sizeof(addr_str_buf_1));
    bt_addr_le_to_str(rpa, addr_str_buf_2, sizeof(addr_str_buf_2));
    LOG_DBG("Identity resolved %s -> %s", log_strdup(addr_str_buf_2), log_strdup(addr_str_buf_1));
}

static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
    (void)conn;
    bt_addr_le_to_str(bt_conn_get_dst(conn), addr_str_buf_1, sizeof(addr_str_buf_1));

    if (err == BT_SECURITY_ERR_SUCCESS) {
        if (level < BT_SECURITY_L2) {
            LOG_WRN("Security change failed: %s level %u", log_strdup(addr_str_buf_1), level);

        } else {
            // セキュリティーレベル変更が成功したら、
            // BLE接続イベントを業務処理スレッドに引き渡す
            LOG_INF("Connected %s with security level %u", log_strdup(addr_str_buf_1), level);
            app_event_notify(APEVT_BLE_CONNECTED);
        }

    } else {
        LOG_WRN("Security failed: %s level %u err %d", log_strdup(addr_str_buf_1), level, err);
    }
}

// 接続時コールバックの設定
BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
    .identity_resolved = identity_resolved,
    .security_changed = security_changed,
};

static void bt_ready(int err)
{
    if (err) {
        // BLE使用不能イベントを業務処理スレッドに引き渡す
        app_event_notify(APEVT_BLE_UNAVAILABLE);
        LOG_ERR("Bluetooth init failed (err %d)", err);
        return;
    }

    // Bluetooth初期処理完了
    LOG_INF("Bluetooth initialized");

    // BLEペアリング用のパスキーを設定
    set_passkey_for_pairing();

    // BLE使用可能イベントを業務処理スレッドに引き渡す
    app_event_notify(APEVT_BLE_AVAILABLE);
}

void app_bluetooth_start(void)
{
    // BLE SMPサービスの設定
    app_ble_smp_register_group();

    // ペアリングモードを設定
    if (app_ble_pairing_mode_set(false) == false) {
        LOG_ERR("Pairing mode set failed");
        return;
    }

    // アドバタイズ処理を work queue に入れる
    k_work_init(&advertise_work, advertise);
    k_work_init(&stop_advertise_work, advertise_stop);

    // Enable Bluetooth.
    //   同時に、内部でNVSの初期化(nvs_init)が行われます。
    int rc = bt_enable(bt_ready);
    if (rc != 0) {
        // BLE使用不能イベントを業務処理スレッドに引き渡す
        app_event_notify(APEVT_BLE_UNAVAILABLE);
        LOG_ERR("Bluetooth init failed (err %d)", rc);
        return;
    }

    // Initialize the Bluetooth mcumgr transport.
    app_ble_smp_bt_register();
}
