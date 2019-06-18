/* 
 * File:   fido_board.c
 * Author: makmorit
 *
 * Created on 2018/12/18, 11:09
 */
#include "sdk_common.h"

// for lighting LED
#include "nrf_gpio.h"

#include "fido_board.h"
#include "fido_timer.h"

// for fido_ble_peripheral_mode
#include "fido_ble_peripheral.h"

// for ble_u2f_pairing_mode_get
#include "ble_u2f_pairing.h"

// LEDの点灯・消灯状態を保持
static bool led_state = false;

// 点滅対象のLEDを保持
// FIDO機能で使用するLEDのピン番号を指定
static uint32_t m_led_for_processing;

// アイドル時LED点滅制御用変数
static uint8_t  led_status = 0;

void fido_led_light_LED(uint32_t pin_number, bool led_on)
{
    // LEDを出力設定
    nrf_gpio_cfg_output(pin_number);
    if (led_on == false) {
        // LEDを点灯させる
        nrf_gpio_pin_set(pin_number);
    } else {
        // LEDを消灯させる
        nrf_gpio_pin_clear(pin_number);
    }
}

void fido_led_light_all_LED(bool led_on)
{
    fido_led_light_LED(LED_FOR_PAIRING_MODE, led_on);
    fido_led_light_LED(LED_FOR_USER_PRESENCE, led_on);
    fido_led_light_LED(LED_FOR_PROCESSING, led_on);
}

void fido_processing_led_timedout_handler(void)
{
    // LEDを点滅させる
    led_state = !led_state;
    fido_led_light_LED(m_led_for_processing, led_state);
}

void fido_processing_led_on(uint32_t led_for_processing, uint32_t on_off_interval_msec)
{
    // 点滅対象のLEDを保持
    m_led_for_processing = led_for_processing;
    
    // タイマーを開始する
    fido_processing_led_timer_start(on_off_interval_msec);
}

void fido_processing_led_off(void)
{
    // LEDを消灯させる
    fido_led_light_LED(m_led_for_processing, false);

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
    fido_led_light_LED(m_led_for_processing, led_state);
}

void fido_idling_led_off(void)
{
    // LEDを消灯させる
    fido_led_light_LED(m_led_for_processing, false);

    // タイマーを停止する
    fido_idling_led_timer_stop();
}

void fido_idling_led_on(void)
{
    // LEDを一旦消灯
    fido_idling_led_off();

    if (ble_u2f_pairing_mode_get()) {
        // ペアリングモードの場合は
        // RED LEDの連続点灯とします。
        m_led_for_processing = LED_FOR_PAIRING_MODE;
        fido_led_light_LED(m_led_for_processing, true);
        return;
    }

    if (fido_ble_peripheral_mode()) {
        // BLEペリフェラル稼働中かつ
        // 非ペアリングモード＝BLUE LED点滅
        m_led_for_processing = LED_FOR_PROCESSING;
    } else {
        // USB HID稼働中＝GREEN LED点滅
        m_led_for_processing = LED_FOR_USER_PRESENCE;
    }
    // タイマーを開始する
    fido_idling_led_timer_start(LED_BLINK_INTERVAL_MSEC);
}
