/* 
 * File:   application_init.c
 * Author: makmorit
 *
 * Created on 2020/08/17, 10:14
 */
#include "sdk_common.h"
#include "app_timer.h"
#include "application_init.h"
#include "ble_service_peripheral.h"

// for nrf_drv_usbd_is_enabled
#include "nrf_drv_usbd.h"

// for fido_button_timers_init
#include "fido_board.h"

// for initialize ATECC608A
#include "atecc.h"

// for logging informations
#define NRF_LOG_MODULE_NAME application_init
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// FIDO Authenticator固有の処理
#include "fido_hid_channel.h"
#include "ctap2_client_pin.h"

//業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// アプリケーション初期化フラグ
//  0: 初期化処理起動待ち
//  1: 初期化処理が実行可能
//  2: 各業務処理が実行可能
//
static APP_INI_STAT application_init_status = APP_INI_STAT_NONE;

APP_INI_STAT application_init_status_get(void)
{
    return application_init_status;
}

void application_init_status_set(APP_INI_STAT s)
{
    application_init_status = s;
}

//
// BLEペリフェラル始動判定用タイマー
//
#define TIMER_MSEC 1000

APP_TIMER_DEF(m_timer_id);
static bool app_timer_created = false;
static bool app_timer_started = false;

static void timeout_handler(void *p_context)
{
    // アプリケーション初期化完了フラグを設定
    // (初期化処理が実行可能)
    application_init_status_set(APP_INI_STAT_EN_INIT);
}

static void timer_terminate(void)
{
    if (app_timer_created == false || app_timer_started == false) {
        return;
    }

    // タイマーを停止
    app_timer_started = false;
    uint32_t err_code = app_timer_stop(m_timer_id);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_stop(m_timer_id) returns %d ", err_code);
    }
}

static bool timer_init(void)
{
    if (app_timer_created == true) {
        return app_timer_created;
    }

    // タイマーを生成
    uint32_t err_code;
    err_code = app_timer_create(&m_timer_id, APP_TIMER_MODE_SINGLE_SHOT, timeout_handler);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_create(timeout_handler) returns %d ", err_code);
        app_timer_created = false;
        return app_timer_created;
    }
    app_timer_created = true;

    // タイマーが既にスタートしている場合は停止させる
    timer_terminate();

    return app_timer_created;
}

static void timer_start(void)
{
    // タイマー生成・停止
    if (timer_init() == false) {
        return;
    }

    // タイマー開始
    uint32_t err_code = app_timer_start(m_timer_id, APP_TIMER_TICKS(TIMER_MSEC), NULL);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_start(m_timer_id) returns %d ", err_code);
        return;
    }
    app_timer_started = true;
}

void application_init_start(void)
{
    // タイマーを開始し、
    // 最初の１秒間でUSB接続されなかった場合は
    // BLEペリフェラル・モードに遷移
    timer_start();
}

static void start_ble_peripheral(void)
{
    // USB接続・HIDサービス始動を確認
    bool enable_usbd = nrf_drv_usbd_is_enabled();
    NRF_LOG_DEBUG("USB HID is %s", 
        enable_usbd ? "active, BLE peripheral is inactive" : "inactive: starting BLE peripheral");

    if (enable_usbd == false) {
        // USB接続・HIDサービスが始動していない場合は
        // アドバタイジングを開始させ、
        // BLEペリフェラル・モードに遷移
        ble_service_peripheral_start();
        return;
    }

    // LED制御をアイドル中（秒間２回点滅）に変更
    fido_status_indicator_idle();
}

void application_init_resume(void)
{
    // アプリケーションで使用するボタンの設定
    fido_button_init();

    // アプリケーションで使用するCIDを初期化
    fido_hid_channel_initialize_cid();

    // PINトークンとキーペアを再生成
    ctap2_client_pin_init();

    // ATECC608A初期化と接続検知
    if (atecc_initialize()) {
        NRF_LOG_INFO("Secure IC was detected: SN(%s)", atecc_get_serial_num_str());
    } else {
        NRF_LOG_INFO("Secure IC was not detected. Flash ROM will be used instead.");
    }

    // BLEペリフェラル・モード遷移可能であれば、
    // BLEペリフェラルを開始
    start_ble_peripheral();

    // アプリケーション初期化完了フラグを設定
    // (各業務処理が実行可能)
    application_init_status_set(APP_INI_STAT_EN_PROC);
    NRF_LOG_INFO("Diverta FIDO Authenticator application started.");
}
