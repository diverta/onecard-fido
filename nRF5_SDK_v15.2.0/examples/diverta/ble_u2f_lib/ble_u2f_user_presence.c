#include "sdk_common.h"

#include <stdio.h>
#include <string.h>
#include "ble_u2f.h"
#include "ble_u2f_util.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ble_u2f_user_presence
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// 機能実行中LED点滅処理
#include "fido_processing_led.h"

// キープアライブ・タイマー
#include "app_timer.h"
#define KEEPALIVE_INTERVAL_MSEC 500

APP_TIMER_DEF(m_ble_u2f_command_timer_id);
static bool app_timer_created = false;

static void command_timer_handler(void *p_context)
{
    // キープアライブ・コマンドを実行する
    ble_u2f_context_t *p_u2f_context = (ble_u2f_context_t *)p_context;
    ble_u2f_send_keepalive_response(p_u2f_context);
}

void ble_u2f_user_presence_init(void)
{
    uint32_t err_code;
    if (app_timer_created == false) {
        err_code = app_timer_create(&m_ble_u2f_command_timer_id, APP_TIMER_MODE_REPEATED, command_timer_handler);
        if (err_code != NRF_SUCCESS) {
            NRF_LOG_ERROR("app_timer_create(m_ble_u2f_command_timer_id) returns %d ", err_code);
            return;
        }
        app_timer_created = true;
    }
}

void ble_u2f_user_presence_terminate(ble_u2f_context_t *p_u2f_context)
{
    if (app_timer_created == false) {
        return;
    }

    // タイマーを停止する
    uint32_t err_code = app_timer_stop(m_ble_u2f_command_timer_id);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_stop(m_ble_u2f_command_timer_id) returns %d ", err_code);
        return;
    }
}

void ble_u2f_user_presence_verify_start(ble_u2f_context_t *p_u2f_context)
{
    if (app_timer_created == false) {
        return;
    }

    // ステータスバイトを設定し、タイマーをスタートする
    uint32_t err_code = app_timer_start(m_ble_u2f_command_timer_id, APP_TIMER_TICKS(KEEPALIVE_INTERVAL_MSEC), p_u2f_context);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_start(m_ble_u2f_command_timer_id) returns %d ", err_code);
        return;
    }

    // LED点滅を開始
    fido_processing_led_on(LED_FOR_USER_PRESENCE, LED_ON_OFF_INTERVAL_MSEC);
    
    NRF_LOG_INFO("User presence verify start ");
}

void ble_u2f_user_presence_verify_end(ble_u2f_context_t *p_u2f_context)
{
    // LEDを消灯させる
    fido_processing_led_off();

    // タイマーを停止する
    ble_u2f_user_presence_terminate(p_u2f_context);
    
    // User presence byte(0x01)を生成
    p_u2f_context->user_presence_byte = 0x01;
    NRF_LOG_INFO("User presence verified ");
}
