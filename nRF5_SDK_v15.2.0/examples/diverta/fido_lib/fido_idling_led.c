/* 
 * File:   fido_idling_led.c
 * Author: makmorit
 *
 * Created on 2018/12/18, 11:09
 */
#include "sdk_common.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_idling_led
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();
#include "nrf_log_ctrl.h"

// タイマー
#include "app_timer.h"
#define LED_ON_OFF_INTERVAL_MSEC 250

// for lighting LED
#include "fido_common.h"

APP_TIMER_DEF(m_idling_led_timer_id);
static bool app_timer_created = false;
static bool led_state = false;

// 点滅対象のLEDを保持
// FIDO機能で使用するLEDのピン番号を指定
static uint32_t m_led_for_processing;
static uint8_t  led_status = 0;

static void command_timer_handler(void *p_context)
{
    // LEDを点滅させる
    // （点滅は約２秒間隔）
    UNUSED_PARAMETER(p_context);
    if (++led_status == 8) {
        led_status = 0;
        led_state = true;
    } else {
        led_state = false;
    }
    fido_led_light_LED(m_led_for_processing, led_state);
}

static void idling_led_init()
{
    uint32_t err_code;
    if (app_timer_created == false) {
        err_code = app_timer_create(&m_idling_led_timer_id, APP_TIMER_MODE_REPEATED, command_timer_handler);
        if (err_code != NRF_SUCCESS) {
            NRF_LOG_ERROR("app_timer_create(m_idling_led_timer_id) returns %d ", err_code);
            return;
        }
        app_timer_created = true;
    }
}

static void idling_led_start()
{
    if (app_timer_created == false) {
        return;
    }

    // タイマーを開始する
    uint32_t err_code = app_timer_start(m_idling_led_timer_id, APP_TIMER_TICKS(LED_ON_OFF_INTERVAL_MSEC), NULL);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_start(m_idling_led_timer_id) returns %d ", err_code);
        return;
    }
}

static void idling_led_terminate()
{
    if (app_timer_created == false) {
        return;
    }

    // タイマーを停止する
    uint32_t err_code = app_timer_stop(m_idling_led_timer_id);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_stop(m_idling_led_timer_id) returns %d ", err_code);
        return;
    }
}

void fido_idling_led_on(uint32_t led_for_processing)
{
    // 点滅対象のLEDを保持
    m_led_for_processing = led_for_processing;
    
    // タイマーが生成されていない場合は生成
    idling_led_init();

    // タイマーを開始する
    idling_led_start();
}

void fido_idling_led_off(uint32_t led_for_processing)
{
    // 点滅対象のLEDを保持
    m_led_for_processing = led_for_processing;
    
    // LEDを消灯させる
    fido_led_light_LED(m_led_for_processing, false);

    // タイマーを停止する
    idling_led_terminate();
}
