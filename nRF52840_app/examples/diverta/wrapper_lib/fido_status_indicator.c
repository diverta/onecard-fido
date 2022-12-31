/* 
 * File:   fido_status_indicator.c
 * Author: makmorit
 *
 * Created on 2022/12/30, 16:22
 */
#include "fido_board.h"
#include "ble_service_common.h"
#include "fido_timer_plat.h"

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

static void processing_led_timed_out(void)
{
    // LEDを点滅させる
    led_state = !led_state;
    fido_board_led_pin_set(m_led_for_processing, led_state);
}

static void idling_led_timed_out(void)
{
    // LEDを点滅させる
    // （点滅は約２秒間隔）
    if (++led_status == 8) {
        led_status = 0;
        led_state = true;
    } else {
        led_state = false;
    }
    fido_board_led_pin_set(m_led_for_idling, led_state);
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
    fido_board_led_pin_set(LED_COLOR_RED,   false);
    fido_board_led_pin_set(LED_COLOR_GREEN, false);
    fido_board_led_pin_set(LED_COLOR_BLUE,  false);
    fido_board_led_pin_set(LED_COLOR_BUSY,  false);
    fido_board_led_pin_set(LED_COLOR_PAIR,  false);
}

void fido_status_indicator_idle(void)
{
    // LED点滅制御タイマーを停止
    stop_led_timers();

    if (ble_service_peripheral_mode()) {
        // LEDを事前消灯
        fido_board_led_pin_set(LED_COLOR_RED,   false);
        fido_board_led_pin_set(LED_COLOR_GREEN, false);
        fido_board_led_pin_set(LED_COLOR_BUSY,  false);
        fido_board_led_pin_set(LED_COLOR_PAIR,  false);

        // BLEペリフェラル稼働中かつ
        // 非ペアリングモード＝BLUE LED点滅
        m_led_for_idling = LED_COLOR_BLUE;

    } else {
        // LEDを事前消灯
        fido_board_led_pin_set(LED_COLOR_RED,   false);
        fido_board_led_pin_set(LED_COLOR_BLUE,  false);
        fido_board_led_pin_set(LED_COLOR_BUSY,  false);
        fido_board_led_pin_set(LED_COLOR_PAIR,  false);

        // USB HID稼働中＝GREEN LED点滅
        m_led_for_idling = LED_COLOR_GREEN;
    }

    // 該当色のLEDを、約２秒ごとに点滅させるタイマーを開始する
    led_status = 0;
    fido_idling_led_timer_start(LED_BLINK_INTERVAL_MSEC, idling_led_timed_out);
}

void fido_status_indicator_busy(void)
{
    // LED点滅制御タイマーを停止
    stop_led_timers();

    // LEDを事前消灯
    fido_board_led_pin_set(LED_COLOR_RED,   false);
    fido_board_led_pin_set(LED_COLOR_GREEN, false);
    fido_board_led_pin_set(LED_COLOR_BLUE,  false);
    fido_board_led_pin_set(LED_COLOR_PAIR,  false);

    // ビジーの場合は
    // 赤色LEDの連続点灯とします。
    fido_board_led_pin_set(LED_COLOR_BUSY,  true);
}

void fido_status_indicator_prompt_reset(void)
{
    // LED点滅制御タイマーを停止
    stop_led_timers();

    // LEDを事前消灯
    fido_board_led_pin_set(LED_COLOR_GREEN, false);
    fido_board_led_pin_set(LED_COLOR_BLUE,  false);
    fido_board_led_pin_set(LED_COLOR_RED,   false);
    fido_board_led_pin_set(LED_COLOR_PAIR,  false);

    // 赤色LEDを、秒間５回点滅させるタイマーを開始する
    m_led_for_processing = LED_COLOR_BUSY;
    fido_processing_led_timer_start(LED_ON_OFF_SHORT_INTERVAL_MSEC, processing_led_timed_out);
}

void fido_status_indicator_ble_scanning(void)
{
    // LED点滅制御タイマーを停止
    stop_led_timers();

    // LEDを事前消灯
    fido_board_led_pin_set(LED_COLOR_GREEN, false);
    fido_board_led_pin_set(LED_COLOR_BLUE,  false);
    fido_board_led_pin_set(LED_COLOR_BUSY,  false);
    fido_board_led_pin_set(LED_COLOR_PAIR,  false);
    
    // 赤色LEDを、秒間２回点滅させるタイマーを開始する
    m_led_for_processing = LED_COLOR_RED;
    fido_processing_led_timer_start(LED_ON_OFF_INTERVAL_MSEC, processing_led_timed_out);
}

void fido_status_indicator_prompt_tup(void)
{
    // LED点滅制御タイマーを停止
    stop_led_timers();

    // LEDを事前消灯
    fido_board_led_pin_set(LED_COLOR_RED,   false);
    fido_board_led_pin_set(LED_COLOR_BLUE,  false);
    fido_board_led_pin_set(LED_COLOR_BUSY,  false);
    fido_board_led_pin_set(LED_COLOR_PAIR,  false);
    
    // 緑色LEDを、秒間２回点滅させるタイマーを開始する
    m_led_for_processing = LED_COLOR_GREEN;
    fido_processing_led_timer_start(LED_ON_OFF_INTERVAL_MSEC, processing_led_timed_out);
}

void fido_status_indicator_pairing_mode(void)
{
    // LED点滅制御タイマーを停止
    stop_led_timers();

    // LEDを事前消灯
    fido_board_led_pin_set(LED_COLOR_RED,   false);
    fido_board_led_pin_set(LED_COLOR_GREEN, false);
    fido_board_led_pin_set(LED_COLOR_BLUE,  false);
    fido_board_led_pin_set(LED_COLOR_BUSY,  false);

    // ペアリングモードの場合は
    // 黄色LEDの連続点灯とします。
    fido_board_led_pin_set(LED_COLOR_PAIR,  true);
}

void fido_status_indicator_pairing_fail(bool short_interval)
{
    // LED点滅制御タイマーを停止
    stop_led_timers();

    // LEDを事前消灯
    fido_board_led_pin_set(LED_COLOR_RED,   false);
    fido_board_led_pin_set(LED_COLOR_GREEN, false);
    fido_board_led_pin_set(LED_COLOR_BLUE,  false);
    fido_board_led_pin_set(LED_COLOR_BUSY,  false);

    // ペアリングモード表示用LEDを点滅させ、
    // 再度ペアリングが必要であることを通知
    //
    // 黄色LEDを、秒間２回点滅させるタイマーを開始する
    m_led_for_processing = LED_COLOR_PAIR;
    if (short_interval) {
        fido_processing_led_timer_start(LED_ON_OFF_SHORT_INTERVAL_MSEC, processing_led_timed_out);
    } else {
        fido_processing_led_timer_start(LED_ON_OFF_INTERVAL_MSEC, processing_led_timed_out);
    }
}

void fido_status_indicator_abort(void)
{
    // LED点滅制御タイマーを停止
    stop_led_timers();

    // LEDを事前消灯
    fido_board_led_pin_set(LED_COLOR_BUSY,  false);
    fido_board_led_pin_set(LED_COLOR_PAIR,  false);

    // 全色LEDを点灯
    fido_board_led_pin_set(LED_COLOR_RED,   true);
    fido_board_led_pin_set(LED_COLOR_GREEN, true);
    fido_board_led_pin_set(LED_COLOR_BLUE,  true);
}
