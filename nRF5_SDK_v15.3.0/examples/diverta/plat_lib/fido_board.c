/* 
 * File:   fido_board.c
 * Author: makmorit
 *
 * Created on 2018/12/18, 11:09
 */
#include "sdk_common.h"

#include "bsp_btn_ble.h"
#include "app_timer.h"
#include "app_button.h"

// for lighting LED
#include "nrf_gpio.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_board
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#include "fido_board.h"
#include "fido_command.h"
#include "fido_timer.h"

// for fido_ble_peripheral_mode
#include "fido_ble_peripheral.h"

// for fido_ble_pairing_mode_get
#include "fido_ble_pairing.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// FIDO機能で使用するLEDのピン番号を設定
// nRF52840 Dongleでは以下の割り当てになります。
//   LED2=Red
//   LED3=Green
//   LED4=Blue
#define LED_FOR_PAIRING_MODE    LED_2
#define LED_FOR_USER_PRESENCE   LED_3
#define LED_FOR_PROCESSING      LED_4

//
// ボタンのピン番号
//
#define PIN_MAIN_SW_IN                  BUTTON_1
#define PIN_MAIN_SW_PULL                BUTTON_PULL

#define APP_BUTTON_NUM                  1
#define APP_BUTTON_DELAY                APP_TIMER_TICKS(100)
#define APP_BUTTON_ACTION_PUSH          APP_BUTTON_PUSH
#define APP_BUTTON_ACTION_RELEASE       APP_BUTTON_RELEASE
#define APP_BUTTON_ACTION_LONG_PUSH     2

#define LONG_PUSH_TIMEOUT               3000

//
// ボタン定義
//
static void on_button_evt(uint8_t pin_no, uint8_t button_action);
static const app_button_cfg_t m_app_buttons[APP_BUTTON_NUM] = {
    {PIN_MAIN_SW_IN, false, PIN_MAIN_SW_PULL, on_button_evt}
};

//
// ボタン長押し検知関連
//
static bool m_long_pushed = false;
static bool m_push_initial = true;

static void on_button_evt(uint8_t pin_no, uint8_t button_action)
{
	switch (button_action) {
	case APP_BUTTON_ACTION_PUSH:
        if (m_push_initial) {
            m_push_initial = false;
        }
        if (pin_no == PIN_MAIN_SW_IN) {
            fido_button_long_push_timer_start(LONG_PUSH_TIMEOUT, NULL);
        }
		break;
		
	case APP_BUTTON_ACTION_RELEASE:
        if (m_push_initial) {
            // ボタン長押し中にリセット後、
            // 単独でこのイベントが発生した場合は無視
            m_push_initial = false;
            break;
        }
        if (m_long_pushed) {
            m_long_pushed = false;
            break;
        }

        if (pin_no == PIN_MAIN_SW_IN) {
            fido_button_long_push_timer_stop();
            
            // FIDO固有の処理を実行
            fido_command_on_mainsw_event();
        }
		break;
		
	case APP_BUTTON_ACTION_LONG_PUSH:
        if (pin_no == PIN_MAIN_SW_IN) {
            // FIDO固有の処理を実行
            fido_command_on_mainsw_long_push_event();
        }
		break;
		
	default:
		break;
	}
}

void fido_command_long_push_timer_handler(void *p_context)
{
    (void)p_context;

	m_long_pushed = true;
	on_button_evt(PIN_MAIN_SW_IN, APP_BUTTON_ACTION_LONG_PUSH);
}

//
// タイマーを追加
//
void fido_button_timers_init(void)
{
    // ボタン長押し検知用タイマー
    fido_button_long_push_timer_init();
}

//
// ボタンをカスタマイズ
//
void fido_button_init(void)
{
    ret_code_t err_code;

    err_code = app_button_init((app_button_cfg_t*)m_app_buttons, APP_BUTTON_NUM, APP_BUTTON_DELAY);
    if (err_code) {
        NRF_LOG_ERROR("app_button_init returns 0x%02x ", err_code);
    }
    APP_ERROR_CHECK(err_code);

    err_code = app_button_enable();
    if (err_code) {
        NRF_LOG_ERROR("app_button_enable returns 0x%02x ", err_code);
    }
    APP_ERROR_CHECK(err_code);
}

//
// LED関連
//
// LEDの点灯・消灯状態を保持
static bool led_state = false;

// 点滅対象のLEDを保持
// FIDO機能で使用するLEDのピン番号を指定
static uint32_t m_led_for_processing;
static uint32_t m_led_for_idling;

// アイドル時LED点滅制御用変数
static uint8_t  led_status = 0;

//
// 処理中表示用LED
//
static void led_light_pin_set(uint32_t pin_number, bool led_on)
{
    // LEDを出力設定
    nrf_gpio_cfg_output(pin_number);
    if (led_on) {
        // LEDを点灯させる
        nrf_gpio_pin_clear(pin_number);
    } else {
        // LEDを消灯させる
        nrf_gpio_pin_set(pin_number);
    }
}

void fido_led_light_all(bool led_on)
{
    led_light_pin_set(LED_FOR_PAIRING_MODE, led_on);
    led_light_pin_set(LED_FOR_USER_PRESENCE, led_on);
    led_light_pin_set(LED_FOR_PROCESSING, led_on);
}

void fido_processing_led_timedout_handler(void)
{
    // LEDを点滅させる
    led_state = !led_state;
    led_light_pin_set(m_led_for_processing, led_state);
}

void fido_prompt_led_blink_start(uint32_t on_off_interval_msec)
{
    // 点滅対象のLEDを保持
    m_led_for_processing = LED_FOR_USER_PRESENCE;
    
    // タイマーを開始する
    fido_processing_led_timer_start(on_off_interval_msec);
}

void fido_caution_led_blink_start(uint32_t on_off_interval_msec)
{
    // 点滅対象のLEDを保持
    m_led_for_processing = LED_FOR_PAIRING_MODE;
    
    // タイマーを開始する
    fido_processing_led_timer_start(on_off_interval_msec);
}

void fido_led_blink_stop(void)
{
    // LEDを消灯させる
    led_light_pin_set(m_led_for_processing, false);

    // タイマーを停止する
    fido_processing_led_timer_stop();
}

void fido_idling_led_timedout_handler(void)
{
    // LEDを点滅させる
    // （点滅は約２秒間隔）
    if (++led_status == 8) {
        led_status = 0;
        led_state = true;
    } else {
        led_state = false;
    }
    led_light_pin_set(m_led_for_idling, led_state);
}

void fido_idling_led_blink_stop(void)
{
    // LEDを消灯させる
    led_light_pin_set(m_led_for_idling, false);

    // タイマーを停止する
    fido_idling_led_timer_stop();
}

void fido_idling_led_blink_start(void)
{
    if (fido_ble_pairing_mode_get()) {
        // ペアリングモードの場合は
        // RED LEDの連続点灯とします。
        m_led_for_idling = LED_FOR_PAIRING_MODE;
        led_light_pin_set(m_led_for_idling, true);
        return;
    }

    if (fido_ble_peripheral_mode()) {
        // BLEペリフェラル稼働中かつ
        // 非ペアリングモード＝BLUE LED点滅
        m_led_for_idling = LED_FOR_PROCESSING;
    } else {
        // USB HID稼働中＝GREEN LED点滅
        m_led_for_idling = LED_FOR_USER_PRESENCE;
    }
    // タイマーを開始する
    fido_idling_led_timer_start(LED_BLINK_INTERVAL_MSEC);
}
