/* 
 * File:   fido_board.cpp
 * Author: makmorit
 *
 * Created on 2019/07/30, 16:18
 */
#include "mbed.h"

#include "fido_board.h"

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

void fido_button_timers_init(void)
{
}

void main_button_fall_handler(void)
{
    // ボタンを押した時の処理
    led_light_pin_set(LED_COLOR_RED, true);
}

void main_button_rise_handler(void)
{
    // ボタンを離した時の処理
    led_light_pin_set(LED_COLOR_RED, false);
}

void fido_button_init(void)
{
    main_button.mode(PullDown);
    main_button.fall(mbed_event_queue()->event(main_button_fall_handler));
    main_button.rise(mbed_event_queue()->event(main_button_rise_handler));
}

void fido_command_long_push_timer_handler(void *p_context)
{
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
