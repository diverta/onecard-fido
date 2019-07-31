/* 
 * File:   fido_board.cpp
 * Author: makmorit
 *
 * Created on 2019/07/30, 16:18
 */
#include "mbed.h"

#include "fido_board.h"
#include "fido_command.h"
#include "fido_log.h"
#include "fido_timer.h"

// 基板上のボタン
InterruptIn main_button(BUTTON1);

// 基板上のLED
#ifdef TARGET_NUCLEO_F411RE
DigitalOut led_green(PA_5);
#else
DigitalOut led_red(PA_4);
DigitalOut led_green(PA_5);
DigitalOut led_blue(PA_6);
#endif

//
// ボタン長押し検知関連
//
#define LONG_PUSH_TIMEOUT 3000
static bool m_long_pushed = false;
static bool m_push_initial = true;

void fido_button_timers_init(void)
{
    // ボタン長押し検知用タイマー
    fido_button_long_push_timer_init();
}

static void main_button_fall_handler(void)
{
    //
    // ボタンを押した時の処理
    //
    if (m_push_initial) {
        m_push_initial = false;
    }

    // 長押し検知タイマーを開始
    fido_button_long_push_timer_start(LONG_PUSH_TIMEOUT, NULL);
}

static void main_button_rise_handler(void)
{
    //
    // ボタンを離した時の処理
    //
    if (m_push_initial) {
        // ボタン長押し中にリセット後、
        // 単独でこのイベントが発生した場合は無視
        m_push_initial = false;
        return;
    }
    if (m_long_pushed) {
        m_long_pushed = false;
        return;
    }

    fido_log_debug("BUTTON1 pushed");

    // 長押し検知タイマーを停止
    fido_button_long_push_timer_stop();
            
    // FIDO固有の処理を実行
    fido_command_mainsw_event_handler();
}

void fido_button_init(void)
{
    main_button.mode(PullDown);
    main_button.fall(mbed_event_queue()->event(main_button_fall_handler));
    main_button.rise(mbed_event_queue()->event(main_button_rise_handler));
}

void fido_command_long_push_timer_handler(void *p_context)
{
    m_long_pushed = true;

    // 現在、割り当てられている機能はありません。
    fido_log_debug("BUTTON1 long pushed");
}

void led_light_pin_set(LED_COLOR led_color, bool led_on)
{
    // led_on
    //  true:  LEDを点灯させる
    //  false: LEDを消灯させる
    int state = led_on ? 1 : 0;

    // 対応するLEDを点灯／消灯
#ifdef TARGET_NUCLEO_F411RE
    led_green = state;
#else
    switch (led_color) {
        case LED_COLOR_RED:
            led_red = state;
            break;
        case LED_COLOR_GREEN:
            led_green = state;
            break;
        case LED_COLOR_BLUE:
            led_blue = state;
            break;
        default:
            return;
    }
#endif
}
