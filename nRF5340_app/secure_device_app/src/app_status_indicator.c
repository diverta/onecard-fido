/* 
 * File:   app_status_indicator.c
 * Author: makmorit
 *
 * Created on 2021/08/23, 10:15
 */
#include <zephyr/types.h>
#include <zephyr.h>

#include "app_board.h"
#include "app_status_indicator.h"

//
// LED点滅制御関連
//
// LED点滅間隔の定義
#define LED_BLINK_OFF_INTERVAL_CNT      3
#define LED_BLINK_ON_INTERVAL_CNT       2
#define LED_BLINK_SHORT_INTERVAL_CNT    1
#define LED_IDLE_OFF_INTERVAL_CNT       18
#define LED_IDLE_ON_INTERVAL_CNT        2

// LEDの点灯・消灯状態を保持
static uint32_t   m_interval_off = 0;
static uint32_t   m_interval_on  = 0;
static uint32_t   m_elapsed_off  = 0;
static uint32_t   m_elapsed_on   = 0;
static bool       m_led_blink    = false;

// 点滅対象のLEDを保持
static LED_COLOR  m_led_for_blink = LED_COLOR_NONE;

static void led_blink_reset(bool blink)
{
    // 内部変数を再設定
    m_elapsed_off = 0;
    m_elapsed_on  = 0;
    m_led_blink   = blink;
}

static void led_blink_parameter_set(LED_COLOR led_for_blink, uint32_t interval_off, uint32_t interval_on)
{
    // 内部変数を再設定
    m_led_for_blink = led_for_blink;
    m_interval_off  = interval_off;
    m_interval_on   = interval_on;
}

//
// USB状態関連
//
// USBが利用可能かどうかを保持
static bool m_usb_available = false;

void app_status_indicator_notify_usb_available(bool available)
{
    // USBが利用可能かどうかを設定
    m_usb_available = available;

    // アイドル時のLED点滅パターンを再設定
    app_status_indicator_idle();
}

bool app_status_indicator_is_usb_available(void)
{
    // USBが利用可能かどうかを戻す
    return m_usb_available;
}

//
// LED点灯／消灯制御関連
//
void app_status_indicator_light_all(bool led_on)
{
    app_board_led_light(LED_COLOR_RED,    led_on);
    app_board_led_light(LED_COLOR_GREEN,  led_on);
    app_board_led_light(LED_COLOR_BLUE,   led_on);
    app_board_led_light(LED_COLOR_YELLOW, led_on);
}

void app_status_indicator_none(void)
{
    // すべてのLEDを消灯
    led_blink_reset(false);
    app_status_indicator_light_all(false);
}

void app_status_indicator_idle(void)
{
    if (m_usb_available) {
        // USB稼働中＝GREEN LED点滅
        led_blink_parameter_set(LED_COLOR_GREEN, LED_IDLE_OFF_INTERVAL_CNT, LED_IDLE_ON_INTERVAL_CNT);

    } else {
        // BLEペリフェラル稼働中かつ
        // 非ペアリングモード＝BLUE LED点滅
        led_blink_parameter_set(LED_COLOR_BLUE, LED_IDLE_OFF_INTERVAL_CNT, LED_IDLE_ON_INTERVAL_CNT);
    }

    // 該当色のLEDを、約２秒ごとに点滅させる
    led_blink_reset(true);
}

void app_status_indicator_busy(void)
{
    // ビジーの場合は
    // 赤色LEDの連続点灯とします。
    led_blink_reset(false);
    app_board_led_light(LED_COLOR_RED,    true);
    app_board_led_light(LED_COLOR_GREEN,  false);
    app_board_led_light(LED_COLOR_BLUE,   false);
    app_board_led_light(LED_COLOR_YELLOW, false);
}

void app_status_indicator_prompt_reset(void)
{
    // 赤色LEDを、秒間５回点滅させる
    led_blink_parameter_set(LED_COLOR_RED, LED_BLINK_SHORT_INTERVAL_CNT, LED_BLINK_SHORT_INTERVAL_CNT);
    led_blink_reset(true);
}

void app_status_indicator_ble_scanning(void)
{
    // 赤色LEDを、秒間２回点滅させる
    led_blink_parameter_set(LED_COLOR_RED, LED_BLINK_OFF_INTERVAL_CNT, LED_BLINK_ON_INTERVAL_CNT);
    led_blink_reset(true);
}

void app_status_indicator_prompt_tup(void)
{
    // 緑色LEDを、秒間２回点滅させる
    led_blink_parameter_set(LED_COLOR_GREEN, LED_BLINK_OFF_INTERVAL_CNT, LED_BLINK_ON_INTERVAL_CNT);
    led_blink_reset(true);
}

void app_status_indicator_pairing_mode(void)
{
    // ペアリングモードの場合は
    // 黄色LEDの連続点灯とします。
    led_blink_reset(false);
    app_board_led_light(LED_COLOR_RED,    false);
    app_board_led_light(LED_COLOR_GREEN,  false);
    app_board_led_light(LED_COLOR_BLUE,   false);
    app_board_led_light(LED_COLOR_YELLOW, true);
}

void app_status_indicator_pairing_fail(void)
{
    // ペアリングモード表示用LEDを点滅させ、
    // 再度ペアリングが必要であることを通知
    //
    // 黄色LEDを、秒間２回点滅させる
    led_blink_parameter_set(LED_COLOR_YELLOW, LED_BLINK_OFF_INTERVAL_CNT, LED_BLINK_ON_INTERVAL_CNT);
    led_blink_reset(true);
}

void app_status_indicator_abort(void)
{
    // 全色LEDを点灯
    led_blink_reset(false);
    app_status_indicator_light_all(true);
}

//
// LED点滅制御
//   100msごとに、タイマースレッドから
//   起動されることを想定しています。
//
void app_status_indicator_blink(void)
{
    // 点滅させる必要がない場合は無視
    if (m_led_blink == false) {
        return;
    }

    // 点滅間隔を制御
    if (m_elapsed_off == 0 && m_elapsed_on == 0) {
        // 全消灯
        app_status_indicator_light_all(false);
        m_elapsed_off++;

    } else if (m_elapsed_off < m_interval_off) {
        m_elapsed_off++;

    } else if (m_elapsed_on < m_interval_on) {
        // 指定のLEDを点灯
        app_board_led_light(m_led_for_blink, true);
        m_elapsed_on++;

    } else {
        // 指定のLEDを消灯
        app_board_led_light(m_led_for_blink, false);
        m_elapsed_off = 1;
        m_elapsed_on = 0;
    }
}
