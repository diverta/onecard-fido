#include "sdk_common.h"
#if NRF_MODULE_ENABLED(BLE_U2F)
#include "ble_u2f.h"
#include "ble_u2f_util.h"

// for logging informations
#define NRF_LOG_MODULE_NAME "ble_u2f_processing_led"
#include "nrf_log.h"

// タイマー
#include "app_timer.h"
#define LED_ON_OFF_INTERVAL_MSEC 300

APP_TIMER_DEF(m_ble_u2f_led_on_off_timer_id);
static bool app_timer_created = false;
static bool led_state = false;

static void command_timer_handler(void *p_context)
{
    // LEDを点滅させる
    ble_u2f_t *p_u2f = (ble_u2f_t *)p_context;
    led_state = !led_state;
    ble_u2f_led_light_LED(p_u2f->led_for_processing_fido, led_state);
}

static void ble_u2f_processing_led_init()
{
    uint32_t err_code;
    if (app_timer_created == false) {
        err_code = app_timer_create(&m_ble_u2f_led_on_off_timer_id, APP_TIMER_MODE_REPEATED, command_timer_handler);
        if (err_code != NRF_SUCCESS) {
            NRF_LOG_ERROR("app_timer_create(m_ble_u2f_led_on_off_timer_id) returns %d \r\n", err_code);
            return;
        }
        app_timer_created = true;
    }
}

static void ble_u2f_processing_led_start(ble_u2f_t *p_u2f)
{
    if (app_timer_created == false) {
        return;
    }

    // タイマーを開始する
    uint32_t err_code = app_timer_start(m_ble_u2f_led_on_off_timer_id, APP_TIMER_TICKS(LED_ON_OFF_INTERVAL_MSEC), p_u2f);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_start(m_ble_u2f_led_on_off_timer_id) returns %d \r\n", err_code);
        return;
    }
}

static void ble_u2f_processing_led_terminate()
{
    if (app_timer_created == false) {
        return;
    }

    // タイマーを停止する
    uint32_t err_code = app_timer_stop(m_ble_u2f_led_on_off_timer_id);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_stop(m_ble_u2f_led_on_off_timer_id) returns %d \r\n", err_code);
        return;
    }
}

void ble_u2f_processing_led_on(ble_u2f_t *p_u2f)
{
    // タイマーが生成されていない場合は生成
    ble_u2f_processing_led_init();

    // タイマーを開始する
    ble_u2f_processing_led_start(p_u2f);
}

void ble_u2f_processing_led_off(ble_u2f_t *p_u2f)
{
    // LEDを消灯させる
    ble_u2f_led_light_LED(p_u2f->led_for_processing_fido, false);

    // タイマーを停止する
    ble_u2f_processing_led_terminate();
}

#endif // NRF_MODULE_ENABLED(BLE_U2F)
