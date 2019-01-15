/* 
 * File:   fido_user_presence.c
 * Author: makmorit
 *
 * Created on 2019/01/15, 10:25
 */
#include "sdk_common.h"

#include <stdio.h>
#include <string.h>

// for logging informations
#define NRF_LOG_MODULE_NAME fido_user_presence
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// 機能実行中LED点滅処理
#include "fido_common.h"
#include "fido_processing_led.h"

// キープアライブ・タイマー
#include "app_timer.h"

APP_TIMER_DEF(m_fido_command_timer_id);
static bool app_timer_created = false;

void fido_user_presence_init(app_timer_timeout_handler_t command_timer_handler)
{
    uint32_t err_code;
    if (app_timer_created == false) {
        err_code = app_timer_create(&m_fido_command_timer_id, APP_TIMER_MODE_REPEATED, command_timer_handler);
        if (err_code != NRF_SUCCESS) {
            NRF_LOG_ERROR("app_timer_create(m_ble_u2f_command_timer_id) returns %d ", err_code);
            return;
        }
        app_timer_created = true;
    }
}

void fido_user_presence_terminate(void)
{
    if (app_timer_created == false) {
        return;
    }

    // タイマーを停止する
    uint32_t err_code = app_timer_stop(m_fido_command_timer_id);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_stop(m_ble_u2f_command_timer_id) returns %d ", err_code);
        return;
    }
}

void fido_user_presence_verify_start(uint32_t timeout_msec, void *p_context)
{
    if (app_timer_created == false) {
        return;
    }

    // ステータスバイトを設定し、タイマーをスタートする
    uint32_t err_code = app_timer_start(m_fido_command_timer_id, APP_TIMER_TICKS(timeout_msec), p_context);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_start(m_ble_u2f_command_timer_id) returns %d ", err_code);
        return;
    }

    // LED点滅を開始
    fido_processing_led_on(LED_FOR_USER_PRESENCE, LED_ON_OFF_INTERVAL_MSEC);
}

uint8_t fido_user_presence_verify_end(void)
{
    // LEDを消灯させる
    fido_processing_led_off();

    // タイマーを停止する
    fido_user_presence_terminate();
    
    // User presence byte(0x01)を生成
    return 0x01;
}
