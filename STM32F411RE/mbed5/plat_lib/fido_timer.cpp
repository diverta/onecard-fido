/* 
 * File:   fido_timer.cpp
 * Author: makmorit
 *
 * Created on 2019/07/31, 11:12
 */
#include "mbed.h"

#include "fido_board.h"
#include "fido_status_indicator.h"

//
// LED点滅タイマー（処理中表示用）
//
Ticker      processing_led_timer;
static bool processing_led_timer_attached = false;

static void processing_led_timeout_handler(void)
{
    fido_processing_led_timedout_handler();
}

void fido_processing_led_timer_stop(void)
{
    // タイマーを停止する
    if (processing_led_timer_attached) {
        processing_led_timer_attached = false;
        processing_led_timer.detach();
    }
}

void fido_processing_led_timer_start(uint32_t on_off_interval_msec)
{
    // すでに開始されている場合は停止
    fido_processing_led_timer_stop();
    
    // タイマーを開始する
    float timeout_sec = on_off_interval_msec / 1000.0;
    processing_led_timer.attach(&processing_led_timeout_handler, timeout_sec);
    processing_led_timer_attached = true;
}

//
// LED点滅タイマー（アイドル時表示用）
//
Ticker      idling_led_timer;
static bool idling_led_timer_attached = false;

static void idling_led_timeout_handler(void)
{
    fido_idling_led_timedout_handler();
}

void fido_idling_led_timer_stop(void)
{
    // タイマーを停止する
    if (idling_led_timer_attached) {
        idling_led_timer_attached = false;
        idling_led_timer.detach();
    }
}

void fido_idling_led_timer_start(uint32_t on_off_interval_msec)
{
    // すでに開始されている場合は停止
    fido_idling_led_timer_stop();
    
    // タイマーを開始する
    float timeout_sec = on_off_interval_msec / 1000.0;
    idling_led_timer.attach(&idling_led_timeout_handler, timeout_sec);
    idling_led_timer_attached = true;
}

//
// ボタン長押し検知用タイマー
//
Timeout     long_push_timer;
static bool long_push_timer_attached = false;

static void button_long_push_timeout_handler(void)
{
    fido_command_long_push_timer_handler(NULL);
}

void fido_button_long_push_timer_init(void)
{
    long_push_timer_attached = false;
}

void fido_button_long_push_timer_stop(void)
{
    // タイマーを停止する
    if (long_push_timer_attached) {
        long_push_timer_attached = false;
        long_push_timer.detach();
    }
}

void fido_button_long_push_timer_start(uint32_t timeout_msec, void *p_context)
{
    // すでに開始されている場合は停止
    fido_button_long_push_timer_stop();
    
    // タイマーを開始する
    float timeout_sec = timeout_msec / 1000.0;
    long_push_timer.attach(&button_long_push_timeout_handler, timeout_sec);
    long_push_timer_attached = true;
}
