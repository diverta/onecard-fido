/* 
 * File:   fido_status_indicator.c
 * Author: makmorit
 *
 * Created on 2019/07/15, 12:27
 */
#include "fido_board.h"
#include "fido_ble_peripheral.h"
#include "fido_timer.h"

//
// LED点滅制御関連
//
// LED点滅間隔の定義
#define LED_ON_OFF_INTERVAL_MSEC        300
#define LED_ON_OFF_SHORT_INTERVAL_MSEC  100
#define LED_BLINK_INTERVAL_MSEC         250

// LEDの点灯・消灯状態を保持
static bool led_state = false;

// 点滅対象のLEDを保持
// FIDO機能で使用するLEDのピン番号を指定
static LED_COLOR m_led_for_processing;
static LED_COLOR m_led_for_idling;

// アイドル時LED点滅制御用変数
static uint8_t led_status = 0;

void fido_processing_led_timedout_handler(void)
{
    // LEDを点滅させる
    led_state = !led_state;
    led_light_pin_set(m_led_for_processing, led_state);
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

static void stop_led_timers(void)
{
    // 処理中LED点滅処理が行われていた場合は
    // タイマーを停止する
    fido_processing_led_timer_stop();

    // アイドル時LED点滅処理が行われていた場合は
    // タイマーを停止する
    fido_idling_led_timer_stop();
}

void fido_status_indicator_none(void)
{
    // LED点滅制御タイマーを停止
    stop_led_timers();

    // すべてのLEDを消灯
    led_light_pin_set(LED_COLOR_RED,   false);
    led_light_pin_set(LED_COLOR_GREEN, false);
    led_light_pin_set(LED_COLOR_BLUE,  false);
    led_light_pin_set(LED_COLOR_BUSY,  false);
    led_light_pin_set(LED_COLOR_PAIR,  false);
}

void fido_status_indicator_idle(void)
{
    // LED点滅制御タイマーを停止
    stop_led_timers();

    if (fido_ble_peripheral_mode()) {
        // LEDを事前消灯
        led_light_pin_set(LED_COLOR_RED,   false);
        led_light_pin_set(LED_COLOR_GREEN, false);
        led_light_pin_set(LED_COLOR_BUSY,  false);
        led_light_pin_set(LED_COLOR_PAIR,  false);

        // BLEペリフェラル稼働中かつ
        // 非ペアリングモード＝BLUE LED点滅
        m_led_for_idling = LED_COLOR_BLUE;

    } else {
        // LEDを事前消灯
        led_light_pin_set(LED_COLOR_RED,   false);
        led_light_pin_set(LED_COLOR_BLUE,  false);
        led_light_pin_set(LED_COLOR_BUSY,  false);
        led_light_pin_set(LED_COLOR_PAIR,  false);

        // USB HID稼働中＝GREEN LED点滅
        m_led_for_idling = LED_COLOR_GREEN;
    }

    // 該当色のLEDを、約２秒ごとに点滅させるタイマーを開始する
    fido_idling_led_timer_start(LED_BLINK_INTERVAL_MSEC);
}

void fido_status_indicator_prompt_reset(void)
{
    // LED点滅制御タイマーを停止
    stop_led_timers();

    // LEDを事前消灯
    led_light_pin_set(LED_COLOR_GREEN, false);
    led_light_pin_set(LED_COLOR_BLUE,  false);
    led_light_pin_set(LED_COLOR_BUSY,  false);
    led_light_pin_set(LED_COLOR_PAIR,  false);

    // 赤色LEDを、秒間５回点滅させるタイマーを開始する
    m_led_for_processing = LED_COLOR_RED;
    fido_processing_led_timer_start(LED_ON_OFF_SHORT_INTERVAL_MSEC);
}

void fido_status_indicator_prompt_tup(void)
{
    // LED点滅制御タイマーを停止
    stop_led_timers();

    // LEDを事前消灯
    led_light_pin_set(LED_COLOR_RED,   false);
    led_light_pin_set(LED_COLOR_BLUE,  false);
    led_light_pin_set(LED_COLOR_BUSY,  false);
    led_light_pin_set(LED_COLOR_PAIR,  false);
    
    // 緑色LEDを、秒間２回点滅させるタイマーを開始する
    m_led_for_processing = LED_COLOR_GREEN;
    fido_processing_led_timer_start(LED_ON_OFF_INTERVAL_MSEC);
}

void fido_status_indicator_pairing_mode(void)
{
    // LED点滅制御タイマーを停止
    stop_led_timers();

    // LEDを事前消灯
    led_light_pin_set(LED_COLOR_RED,   false);
    led_light_pin_set(LED_COLOR_GREEN, false);
    led_light_pin_set(LED_COLOR_BLUE,  false);
    led_light_pin_set(LED_COLOR_BUSY,  false);

    // ペアリングモードの場合は
    // 黄色LEDの連続点灯とします。
    led_light_pin_set(LED_COLOR_PAIR,  true);
}

void fido_status_indicator_pairing_fail(void)
{
    // LED点滅制御タイマーを停止
    stop_led_timers();

    // LEDを事前消灯
    led_light_pin_set(LED_COLOR_RED,   false);
    led_light_pin_set(LED_COLOR_GREEN, false);
    led_light_pin_set(LED_COLOR_BLUE,  false);
    led_light_pin_set(LED_COLOR_BUSY,  false);

    // ペアリングモード表示用LEDを点滅させ、
    // 再度ペアリングが必要であることを通知
    //
    // 黄色LEDを、秒間２回点滅させるタイマーを開始する
    m_led_for_processing = LED_COLOR_PAIR;
    fido_processing_led_timer_start(LED_ON_OFF_INTERVAL_MSEC);
}

void fido_status_indicator_abort(void)
{
    // LED点滅制御タイマーを停止
    stop_led_timers();

    // LEDを事前消灯
    led_light_pin_set(LED_COLOR_BUSY,  false);
    led_light_pin_set(LED_COLOR_PAIR,  false);

    // 全色LEDを点灯
    led_light_pin_set(LED_COLOR_RED,   true);
    led_light_pin_set(LED_COLOR_GREEN, true);
    led_light_pin_set(LED_COLOR_BLUE,  true);
}
