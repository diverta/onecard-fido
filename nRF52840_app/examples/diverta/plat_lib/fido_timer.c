/* 
 * File:   fido_timer.c
 * Author: makmorit
 *
 * Created on 2019/06/18, 11:00
 */
#include "sdk_common.h"
#include "app_timer.h"

#include "fido_ble_event.h"
#include "fido_board.h"
#include "fido_command.h"
#include "fido_log.h"
#include "fido_status_indicator.h"

#include "fido_hid_channel.h"

//
// 無通信タイマー
//
#define COMMUNICATION_INTERVAL_MSEC 10000
APP_TIMER_DEF(m_comm_interval_timer_id);
static bool comm_interval_timer_created = false;

static void comm_interval_timeout_handler(void *p_context)
{
    // BLE接続が行われていた場合は、切断等の処理を行う
    fido_ble_on_process_timedout();
}

static ret_code_t comm_interval_timer_init(void)
{
    if (comm_interval_timer_created) {
        return NRF_SUCCESS;
    }

    // 直近レスポンスからの経過秒数監視するためのタイマーを生成
    ret_code_t err_code = app_timer_create(&m_comm_interval_timer_id, APP_TIMER_MODE_SINGLE_SHOT, comm_interval_timeout_handler);
    if (err_code != NRF_SUCCESS) {
        fido_log_error("app_timer_create(m_comm_interval_timer_id) returns %d ", err_code);
    }
    
    comm_interval_timer_created = true;
    return err_code;
}

void fido_comm_interval_timer_stop(void)
{
    // 直近レスポンスからの経過秒数監視を停止
    app_timer_stop(m_comm_interval_timer_id);
}

void fido_comm_interval_timer_start(void)
{
    // タイマー生成
    ret_code_t err_code = comm_interval_timer_init();
    if (err_code != NRF_SUCCESS) {
        return;
    }

    // タイマーが既にスタートしている場合は停止させる
    fido_comm_interval_timer_stop();

    // 直近レスポンスからの経過秒数監視を開始
    err_code = app_timer_start(m_comm_interval_timer_id, APP_TIMER_TICKS(COMMUNICATION_INTERVAL_MSEC), NULL);
    if (err_code != NRF_SUCCESS) {
        fido_log_error("app_timer_start(m_comm_interval_timer_id) returns %d ", err_code);
    }
}

//
// ユーザー所在確認タイムアウト監視用タイマー（３０秒）
//
APP_TIMER_DEF(m_user_presence_verify_timer_id);
static bool user_presence_verify_timer_created = false;

static void user_presence_verify_timeout_handler(void *p_context)
{
    // ユーザー所在確認タイムアウト時の処理を実行
    fido_user_presence_verify_timeout_handler();
}

static ret_code_t process_timeout_timer_init(void)
{
    if (user_presence_verify_timer_created) {
        return NRF_SUCCESS;
    }

    // ユーザー所在確認タイムアウト監視用タイマーを生成
    ret_code_t err_code = app_timer_create(&m_user_presence_verify_timer_id, APP_TIMER_MODE_SINGLE_SHOT, user_presence_verify_timeout_handler);
    if (err_code != NRF_SUCCESS) {
        fido_log_error("app_timer_create(m_user_presence_verify_timer_id) returns %d ", err_code);
    }
    
    user_presence_verify_timer_created = true;
    return err_code;
}

void fido_user_presence_verify_timer_stop(void)
{
    // ユーザー所在確認タイムアウト監視用タイマーを停止
    app_timer_stop(m_user_presence_verify_timer_id);
}

void fido_user_presence_verify_timer_start(uint32_t timeout_msec, void *p_context)
{
    // タイマー生成
    ret_code_t err_code = process_timeout_timer_init();
    if (err_code != NRF_SUCCESS) {
        return;
    }

    // タイマーが既にスタートしている場合は停止させる
    fido_user_presence_verify_timer_stop();

    // ユーザー所在確認タイムアウト監視用タイマーを停止
    err_code = app_timer_start(m_user_presence_verify_timer_id, APP_TIMER_TICKS(timeout_msec), p_context);
    if (err_code != NRF_SUCCESS) {
        fido_log_error("app_timer_start(m_user_presence_verify_timer_id) returns %d ", err_code);
    }
}

//
// LED点滅タイマー（処理中表示用）
//
APP_TIMER_DEF(m_led_on_off_timer_id);
static bool led_on_off_timer_created = false;

static void processing_led_timeout_handler(void *p_context)
{
    fido_processing_led_timedout_handler();
}

static ret_code_t processing_led_timer_init()
{
    if (led_on_off_timer_created) {
        return NRF_SUCCESS;
    }

    ret_code_t err_code = app_timer_create(&m_led_on_off_timer_id, APP_TIMER_MODE_REPEATED, processing_led_timeout_handler);
    if (err_code != NRF_SUCCESS) {
        fido_log_error("app_timer_create(m_led_on_off_timer_id) returns %d ", err_code);
    }
    
    led_on_off_timer_created = true;
    return err_code;
}

void fido_processing_led_timer_stop(void)
{
    // タイマーを停止する
    app_timer_stop(m_led_on_off_timer_id);
}

void fido_processing_led_timer_start(uint32_t on_off_interval_msec)
{
    // タイマー生成
    ret_code_t err_code = processing_led_timer_init();
    if (err_code != NRF_SUCCESS) {
        return;
    }

    // タイマーが既にスタートしている場合は停止させる
    fido_processing_led_timer_stop();

    // タイマーを開始する
    err_code = app_timer_start(m_led_on_off_timer_id, APP_TIMER_TICKS(on_off_interval_msec), NULL);
    if (err_code != NRF_SUCCESS) {
        fido_log_error("app_timer_start(m_led_on_off_timer_id) returns %d ", err_code);
    }
}

//
// LED点滅タイマー（アイドル時表示用）
//
APP_TIMER_DEF(m_idling_led_timer_id);
static bool idling_led_timer_created = false;

static void idling_led_timeout_handler(void *p_context)
{
    fido_idling_led_timedout_handler();
}

static ret_code_t idling_led_timer_init()
{
    if (idling_led_timer_created) {
        return NRF_SUCCESS;
    }

    ret_code_t err_code = app_timer_create(&m_idling_led_timer_id, APP_TIMER_MODE_REPEATED, idling_led_timeout_handler);
    if (err_code != NRF_SUCCESS) {
        fido_log_error("app_timer_create(m_idling_led_timer_id) returns %d ", err_code);
    }
    
    idling_led_timer_created = true;
    return err_code;
}

void fido_idling_led_timer_stop(void)
{
    // タイマーを停止する
    app_timer_stop(m_idling_led_timer_id);
}

void fido_idling_led_timer_start(uint32_t on_off_interval_msec)
{
    // タイマー生成
    ret_code_t err_code = idling_led_timer_init();
    if (err_code != NRF_SUCCESS) {
        return;
    }

    // タイマーが既にスタートしている場合は停止させる
    fido_idling_led_timer_stop();

    // タイマーを開始する
    err_code = app_timer_start(m_idling_led_timer_id, APP_TIMER_TICKS(on_off_interval_msec), NULL);
    if (err_code != NRF_SUCCESS) {
        fido_log_error("app_timer_start(m_idling_led_timer_id) returns %d ", err_code);
    }
}

//
// ボタン長押し検知用タイマー
//
APP_TIMER_DEF(m_long_push_timer_id);
static bool long_push_timer_created = false;

static void button_long_push_timeout_handler(void *p_context)
{
    fido_command_long_push_timer_handler(p_context);
}

//
// タイマーを追加
//
void fido_button_long_push_timer_init(void)
{
    if (long_push_timer_created) {
        return;
    }

    // ボタン長押し検知用タイマー（５秒）
    ret_code_t err_code = app_timer_create(&m_long_push_timer_id, APP_TIMER_MODE_SINGLE_SHOT, button_long_push_timeout_handler);
    if (err_code != NRF_SUCCESS) {
        fido_log_error("app_timer_create(m_long_push_timer_id) returns %d ", err_code);
    }

    long_push_timer_created = true;
}

void fido_button_long_push_timer_stop(void)
{
    // タイマーを停止する
    app_timer_stop(m_long_push_timer_id);
}

void fido_button_long_push_timer_start(uint32_t timeout_msec, void *p_context)
{
    // タイマーを開始する
    ret_code_t err_code = app_timer_start(m_long_push_timer_id, APP_TIMER_TICKS(timeout_msec), p_context);
    if (err_code != NRF_SUCCESS) {
        fido_log_error("app_timer_start(m_long_push_timer_id) returns %d ", err_code);
    }
}

//
// HIDチャネルロック用タイマー
// 
APP_TIMER_DEF(m_hid_channel_lock_timer_id);
static bool hid_channel_lock_timer_created = false;

static void hid_channel_lock_timeout_handler(void *p_context)
{
    fido_hid_channel_lock_timedout_handler();
}

static ret_code_t hid_channel_lock_timer_init(void)
{
    if (hid_channel_lock_timer_created) {
        return NRF_SUCCESS;
    }

    // 直近レスポンスからの経過秒数監視するためのタイマーを生成
    ret_code_t err_code;
    err_code = app_timer_create(&m_hid_channel_lock_timer_id, APP_TIMER_MODE_SINGLE_SHOT, hid_channel_lock_timeout_handler);
    if (err_code != NRF_SUCCESS) {
        fido_log_error("app_timer_create(m_hid_channel_lock_timer_id) returns %d ", err_code);
    }
    
    hid_channel_lock_timer_created = true;
    return err_code;
}

void fido_hid_channel_lock_timer_stop(void)
{
    // 直近レスポンスからの経過秒数監視を停止
    app_timer_stop(m_hid_channel_lock_timer_id);
}

void fido_hid_channel_lock_timer_start(uint32_t lock_ms)
{
    // タイマー生成
    ret_code_t err_code = hid_channel_lock_timer_init();
    if (err_code != NRF_SUCCESS) {
        return;
    }

    // タイマーが既にスタートしている場合は停止させる
    fido_hid_channel_lock_timer_stop();

    // 直近レスポンスからの経過秒数監視を開始
    err_code = app_timer_start(m_hid_channel_lock_timer_id, APP_TIMER_TICKS(lock_ms), NULL);
    if (err_code != NRF_SUCCESS) {
        fido_log_error("app_timer_start(m_hid_channel_lock_timer_id) returns %d ", err_code);
    }
}

//
// 反復処理汎用タイマー
// 
APP_TIMER_DEF(m_repeat_process_timer_id);
static bool repeat_process_timer_created = false;
static void (*repeat_process_handler)(void);

static void repeat_process_timeout_handler(void *p_context)
{
    // ハンドラーを実行する
    (void)p_context;
    (*repeat_process_handler)();
}

static ret_code_t repeat_process_timer_init(void)
{
    if (repeat_process_timer_created) {
        return NRF_SUCCESS;
    }

    ret_code_t err_code = app_timer_create(&m_repeat_process_timer_id, APP_TIMER_MODE_REPEATED, repeat_process_timeout_handler);
    if (err_code != NRF_SUCCESS) {
        fido_log_error("app_timer_create(m_repeat_process_timer_id) returns %d ", err_code);
    }
    
    repeat_process_timer_created = true;
    return err_code;
}

void fido_repeat_process_timer_stop(void)
{
    // タイマーを停止する
    app_timer_stop(m_repeat_process_timer_id);
}

void fido_repeat_process_timer_start(uint32_t timeout_msec, void (*handler)(void))
{
    // タイマー生成
    ret_code_t err_code = repeat_process_timer_init();
    if (err_code != NRF_SUCCESS) {
        return;
    }

    // タイマーが既にスタートしている場合は停止させる
    fido_repeat_process_timer_stop();

    // ハンドラーの参照を保持
    repeat_process_handler = handler;
    
    // タイマーを開始する
    err_code = app_timer_start(m_repeat_process_timer_id, APP_TIMER_TICKS(timeout_msec), NULL);
    if (err_code != NRF_SUCCESS) {
        fido_log_error("app_timer_start(m_repeat_process_timer_id) returns %d ", err_code);
    }
}
