/* 
 * File:   fido_ble_peripheral_timer.c
 * Author: makmorit
 *
 * Created on 2019/03/11, 11:13
 */
#include "sdk_common.h"
#include "app_timer.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_ble_peripheral_timer
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for nrf_drv_usbd_is_enabled
#include "nrf_drv_usbd.h"

// for fido_ble_peripheral_advertising_start
#include "fido_ble_peripheral.h"

// for lighting LED
#include "fido_board.h"

#define TIMER_MSEC 1000

// タイマー
APP_TIMER_DEF(m_timer_id);
static bool app_timer_created = false;
static bool app_timer_started = false;

static void timeout_handler(void *p_context)
{
    UNUSED_PARAMETER(p_context);

    // USB接続・HIDサービス始動を確認
    bool enable_usbd = nrf_drv_usbd_is_enabled();
    NRF_LOG_DEBUG("USB HID is %s", 
        enable_usbd ? "active, BLE peripheral is inactive" : "inactive: starting BLE peripheral");

    if (enable_usbd == false) {
        // USB接続・HIDサービスが始動していない場合は
        // アドバタイジングを開始させ、
        // BLEペリフェラル・モードに遷移
        fido_ble_peripheral_advertising_start();
    }

    // アイドル時点滅処理を開始
    // USB HID、BLEでアイドル時のLED色を変える
    fido_idling_led_on();
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

void fido_ble_peripheral_timer_start(void)
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
