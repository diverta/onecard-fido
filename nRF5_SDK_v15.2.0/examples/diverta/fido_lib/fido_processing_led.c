/* 
 * File:   fido_processing_led.c
 * Author: makmorit
 *
 * Created on 2018/12/18, 12:09
 */
#include "sdk_common.h"

// for lighting LED
#include "fido_common.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_processing_led
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// タイマー
#include "app_timer.h"

APP_TIMER_DEF(m_led_on_off_timer_id);
static bool app_timer_created = false;
static bool led_state = false;

// 点滅対象のLEDを保持
// FIDO機能で使用するLEDのピン番号を指定
static uint32_t m_led_for_processing;

static void command_timer_handler(void *p_context)
{
    // LEDを点滅させる
    led_state = !led_state;
    fido_led_light_LED(m_led_for_processing, led_state);
}

static void processing_led_init()
{
    uint32_t err_code;
    if (app_timer_created == false) {
        err_code = app_timer_create(&m_led_on_off_timer_id, APP_TIMER_MODE_REPEATED, command_timer_handler);
        if (err_code != NRF_SUCCESS) {
            NRF_LOG_ERROR("app_timer_create(m_led_on_off_timer_id) returns %d ", err_code);
            return;
        }
        app_timer_created = true;
    }
}

static void processing_led_start(uint32_t on_off_interval_msec)
{
    if (app_timer_created == false) {
        return;
    }

    // タイマーを開始する
    uint32_t err_code = app_timer_start(m_led_on_off_timer_id, APP_TIMER_TICKS(on_off_interval_msec), NULL);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_start(m_led_on_off_timer_id) returns %d ", err_code);
        return;
    }
}

static void processing_led_terminate()
{
    if (app_timer_created == false) {
        return;
    }

    // タイマーを停止する
    uint32_t err_code = app_timer_stop(m_led_on_off_timer_id);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_stop(m_led_on_off_timer_id) returns %d ", err_code);
        return;
    }
}

void fido_processing_led_on(uint32_t led_for_processing, uint32_t on_off_interval_msec)
{
    // 点滅対象のLEDを保持
    m_led_for_processing = led_for_processing;
    
    // タイマーが生成されていない場合は生成
    processing_led_init();

    // タイマーを開始する
    processing_led_start(on_off_interval_msec);
}

void fido_processing_led_off(void)
{
    // LEDを消灯させる
    fido_led_light_LED(m_led_for_processing, false);

    // タイマーを停止する
    processing_led_terminate();
}
