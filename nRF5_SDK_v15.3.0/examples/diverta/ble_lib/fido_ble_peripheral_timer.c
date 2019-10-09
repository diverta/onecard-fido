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

#include "ble_service_common.h"

#define TIMER_MSEC 1000

// タイマー
APP_TIMER_DEF(m_timer_id);
static bool app_timer_created = false;
static bool app_timer_started = false;

static void timeout_handler(void *p_context)
{
    ble_service_common_start_peripheral(p_context);
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
