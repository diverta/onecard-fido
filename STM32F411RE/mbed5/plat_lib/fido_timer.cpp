/* 
 * File:   fido_timer.cpp
 * Author: makmorit
 *
 * Created on 2019/07/31, 11:12
 */
#include "mbed.h"

#include "fido_board.h"
#include "fido_command.h"
#include "fido_hid_channel.h"
#include "fido_status_indicator.h"

static bool m_fido_processing_led_timeout;
static bool m_idling_led_timeout;
static bool m_button_long_push_timeout;
static bool m_user_presence_verify_timeout;
static bool m_keepalive_interval_timeout;
static bool m_hid_channel_lock_timeout;

//
// mainループ内から呼び出される処理
//
void fido_timer_do_process(void)
{
    if (m_fido_processing_led_timeout) {
        m_fido_processing_led_timeout = false;
        fido_processing_led_timedout_handler();
    }
    
    if (m_idling_led_timeout) {
        m_idling_led_timeout = false;
        fido_idling_led_timedout_handler();
    }
    
    if (m_button_long_push_timeout) {
        m_button_long_push_timeout = false;
        fido_command_long_push_timer_handler(NULL);
    }

    if (m_user_presence_verify_timeout) {
        m_user_presence_verify_timeout = false;
        // ユーザー所在確認タイムアウト時の処理を実行
        fido_user_presence_verify_timeout_handler();
    }

    if (m_keepalive_interval_timeout) {
        m_keepalive_interval_timeout = false;
        // キープアライブレスポンスを送信
        fido_command_keepalive_timer_handler();
    }

    if (m_hid_channel_lock_timeout) {
        m_hid_channel_lock_timeout = false;
        // HIDチャネルロックタイムアウト時の処理を実行
        fido_hid_channel_lock_timedout_handler(NULL);
    }
}

//
// LED点滅タイマー（処理中表示用）
//
Ticker      processing_led_timer;
static bool processing_led_timer_attached = false;

static void processing_led_timeout_handler(void)
{
    m_fido_processing_led_timeout = true;
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
    m_idling_led_timeout = true;
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
    m_button_long_push_timeout = true;
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

//
// ユーザー所在確認タイムアウト監視用タイマー（３０秒）
//
Timeout     user_presence_verify_timer;
static bool user_presence_verify_timer_attached = false;

static void user_presence_verify_timeout_handler(void)
{
    m_user_presence_verify_timeout = true;
}

void _fido_user_presence_verify_timer_stop(void)
{
    // タイマーを停止する
    if (user_presence_verify_timer_attached) {
        user_presence_verify_timer_attached = false;
        user_presence_verify_timer.detach();
    }
}

void _fido_user_presence_verify_timer_start(uint32_t timeout_msec, void *p_context)
{
    // すでに開始されている場合は停止
    _fido_user_presence_verify_timer_stop();
    
    // タイマーを開始する
    float timeout_sec = timeout_msec / 1000.0;
    user_presence_verify_timer.attach(&user_presence_verify_timeout_handler, timeout_sec);
    user_presence_verify_timer_attached = true;
}

//
// キープアライブタイマー
//
Ticker      keepalive_interval_timer;
static bool keepalive_interval_timer_attached = false;

static void keepalive_interval_timeout_handler(void)
{
    m_keepalive_interval_timeout = true;
}

void _fido_keepalive_interval_timer_stop(void)
{
    // タイマーを停止する
    if (keepalive_interval_timer_attached) {
        keepalive_interval_timer_attached = false;
        keepalive_interval_timer.detach();
    }
}

void _fido_keepalive_interval_timer_start(uint32_t timeout_msec, void *p_context)
{
    // すでに開始されている場合は停止
    _fido_keepalive_interval_timer_stop();
    
    // タイマーを開始する
    float timeout_sec = timeout_msec / 1000.0;
    keepalive_interval_timer.attach(&keepalive_interval_timeout_handler, timeout_sec);
    keepalive_interval_timer_attached = true;
}

//
// HIDチャネルロックタイムアウト監視用タイマー（最大10秒）
//
Timeout     hid_channel_lock_timer;
static bool hid_channel_lock_timer_attached = false;

static void hid_channel_lock_timeout_handler(void)
{
    m_hid_channel_lock_timeout = true;
}

void _fido_hid_channel_lock_timer_stop(void)
{
    // タイマーを停止する
    if (hid_channel_lock_timer_attached) {
        hid_channel_lock_timer_attached = false;
        hid_channel_lock_timer.detach();
    }
}

void _fido_hid_channel_lock_timer_start(uint32_t lock_ms)
{
    // すでに開始されている場合は停止
    _fido_hid_channel_lock_timer_stop();
    
    // タイマーを開始する
    float timeout_sec = lock_ms / 1000.0;
    hid_channel_lock_timer.attach(&hid_channel_lock_timeout_handler, timeout_sec);
    hid_channel_lock_timer_attached = true;
}
