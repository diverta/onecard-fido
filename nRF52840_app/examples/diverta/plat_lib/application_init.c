/* 
 * File:   application_init.c
 * Author: makmorit
 *
 * Created on 2020/08/17, 10:14
 */
#include "sdk_common.h"
#include "app_timer.h"
#include "application_init_define.h"
#include "ble_service_peripheral.h"

// for nrf_drv_usbd_is_enabled
#include "nrf_drv_usbd.h"
#include "app_usbd.h"

// for fido_button_timers_init
#include "fido_board.h"

#if !defined(NO_SECURE_IC)
// for initialize ATECC608A
#include "atecc.h"
#endif

// for RTCC/TFT module use
#include "rtcc.h"
#include "tiny_tft.h"

// for logging informations
#define NRF_LOG_MODULE_NAME application_init
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// FIDO Authenticator固有の処理
#include "fido_hid_channel.h"
#include "ctap2_client_pin.h"
#include "fido_ble_event.h"
#include "fido_ble_pairing.h"
#include "usbd_service.h"

//業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

static APP_INI_STAT application_init_status = APP_INI_STAT_NONE;

APP_TIMER_DEF(m_timer_id);
static bool app_timer_created = false;
static bool app_timer_started = false;

static void timeout_handler(void *p_context)
{
    // アプリケーション初期化完了フラグを設定
    // (BLEペリフェラルモード遷移判定処理が実行可能)
    application_init_status = APP_INI_STAT_EN_BLEADV;
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
    uint32_t err_code = app_timer_start(m_timer_id, APP_TIMER_TICKS(BLE_ADVERTISE_START_TIMER_MSEC), NULL);
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
        if (fido_ble_pairing_sleep_after_boot_mode()) {
            // ペアリングモードレコードが存在していない場合、
            // スリープ状態に遷移
            //   ボタン押下でアイドル状態に復帰できるよう、
            //   アプリケーションで使用するボタンを事前に設定
            fido_board_button_init();
            fido_board_prepare_for_deep_sleep();
            return;
        }

        // USB接続・HIDサービスが始動していない場合は
        // アドバタイジングを開始させ、
        // BLEペリフェラル・モードに遷移
        ble_service_peripheral_start();

        // アプリケーション初期化完了フラグを設定
        // (初期化処理が実行可能)
        application_init_status = APP_INI_STAT_EN_INIT;

    } else {
        // ペアリングモードレコードをFlash ROMから削除
        //   USB接続が解除-->システムのリスタート時
        //   BLEアイドル状態に遷移するのを抑止するための措置
        fido_ble_pairing_reset();
    }
}

void application_init_ble_pairing_has_reset(void)
{
    // Flash ROMからペアリングモード削除後の処理
    // LED制御をアイドル中（秒間２回点滅）に変更
    fido_status_indicator_idle();

    // アプリケーション初期化完了フラグを設定
    // (初期化処理が実行可能)
    application_init_status = APP_INI_STAT_EN_INIT;
}

static void application_init_resume(void)
{
    // アプリケーションで使用するボタンの設定
    fido_board_button_init();

    // アプリケーションで使用するCIDを初期化
    fido_hid_channel_initialize_cid();

    // PINトークンとキーペアを再生成
    ctap2_client_pin_init();

#if defined(NO_SECURE_IC)
    NRF_LOG_INFO("Secure IC is not installed. Flash ROM will be used instead.");
#else
    // ATECC608A初期化と接続検知
    if (atecc_initialize()) {
        NRF_LOG_INFO("Secure IC was detected: SN(%s)", atecc_get_serial_num_str());
    } else {
        NRF_LOG_INFO("Secure IC was not detected. Flash ROM will be used instead.");
    }
#endif

#if defined(NO_RTCC_MODULE)
    NRF_LOG_INFO("Realtime clock calendar is not installed.");
#else
    // RTCC初期化と接続検知
    rtcc_init();
#endif

#if defined(NO_TTFT_MODULE)
    NRF_LOG_INFO("TFT display is not installed.");
#else
    // TFTディスプレイを初期化
    tiny_tft_init_display();
#endif

    // アプリケーション初期化完了フラグを設定
    // (各業務処理が実行可能)
    application_init_status = APP_INI_STAT_EN_PROC;
    NRF_LOG_INFO("Diverta FIDO Authenticator application started: %s Version %s (%s)", HW_REV, FW_REV, FW_BUILD);
}

void application_main(void)
{
    // USBデバイス処理を実行
    while (app_usbd_event_queue_process());

    switch (application_init_status) {
        case APP_INI_STAT_EN_BLEADV:
            // BLEペリフェラルモード遷移判定処理
            start_ble_peripheral();
            break;
        case APP_INI_STAT_EN_INIT:
            // アプリケーション稼働に必要な初期化処理を再開
            application_init_resume();    
            break;
        case APP_INI_STAT_EN_PROC:
            // 業務処理を実行
            usbd_service_do_process();
            fido_ble_do_process();
            break;
        default:
            break;
    }
}
