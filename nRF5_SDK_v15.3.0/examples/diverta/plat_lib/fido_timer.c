/* 
 * File:   fido_timer.c
 * Author: makmorit
 *
 * Created on 2019/06/18, 11:00
 */
#include "sdk_common.h"
#include "app_timer.h"

#include "fido_board.h"
#include "fido_command.h"
#include "fido_log.h"

#include "usbd_hid_common.h"

//
// 無通信タイマー
//
#define COMMUNICATION_INTERVAL_MSEC 10000
APP_TIMER_DEF(m_comm_interval_timer_id);
static bool comm_interval_timer_created = false;

static void comm_interval_timeout_handler(void *p_context)
{
    // 直近のレスポンスから10秒を経過した場合、
    // FIDO機能処理タイムアウト時の処理を実行
    fido_command_on_process_timedout();
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
// ユーザー所在確認中のキープアライブ用タイマー
//
APP_TIMER_DEF(m_keepalive_interval_timer_id);
static bool keepalive_interval_timer_created = false;

static void keepalive_interval_timeout_handler(void *p_context)
{
    // キープアライブ・コマンドを実行する
    fido_command_keepalive_timer_handler(p_context);
}

static ret_code_t keepalive_interval_timer_init(void)
{
    if (keepalive_interval_timer_created) {
        return NRF_SUCCESS;
    }

    ret_code_t err_code = app_timer_create(&m_keepalive_interval_timer_id, APP_TIMER_MODE_REPEATED, keepalive_interval_timeout_handler);
    if (err_code != NRF_SUCCESS) {
        fido_log_error("app_timer_create(m_keepalive_interval_timer_id) returns %d ", err_code);
    }
    
    keepalive_interval_timer_created = true;
    return err_code;
}

void fido_keepalive_interval_timer_stop(void)
{
    // タイマーを停止する
    app_timer_stop(m_keepalive_interval_timer_id);
}

void fido_keepalive_interval_timer_start(uint32_t timeout_msec, void *p_context)
{
    // タイマー生成
    ret_code_t err_code = keepalive_interval_timer_init();
    if (err_code != NRF_SUCCESS) {
        return;
    }

    // タイマーが既にスタートしている場合は停止させる
    fido_keepalive_interval_timer_stop();

    // タイマーを開始する
    err_code = app_timer_start(m_keepalive_interval_timer_id, APP_TIMER_TICKS(timeout_msec), p_context);
    if (err_code != NRF_SUCCESS) {
        fido_log_error("app_timer_start(m_keepalive_interval_timer_id) returns %d ", err_code);
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
APP_TIMER_DEF(m_lock_channel_timer_id);
static bool lock_channel_timer_created = false;

static void lock_channel_timeout_handler(void *p_context)
{
    fido_lock_channel_timedout_handler(p_context);
}

static ret_code_t lock_channel_timer_init(void)
{
    if (lock_channel_timer_created) {
        return NRF_SUCCESS;
    }

    // 直近レスポンスからの経過秒数監視するためのタイマーを生成
    ret_code_t err_code;
    err_code = app_timer_create(&m_lock_channel_timer_id, APP_TIMER_MODE_SINGLE_SHOT, lock_channel_timeout_handler);
    if (err_code != NRF_SUCCESS) {
        fido_log_error("app_timer_create(m_lock_channel_timer_id) returns %d ", err_code);
    }
    
    lock_channel_timer_created = true;
    return err_code;
}

void fido_lock_channel_timer_stop(void)
{
    // 直近レスポンスからの経過秒数監視を停止
    app_timer_stop(m_lock_channel_timer_id);
}

void fido_lock_channel_timer_start(uint32_t lock_ms)
{
    // タイマー生成
    ret_code_t err_code = lock_channel_timer_init();
    if (err_code != NRF_SUCCESS) {
        return;
    }

    // タイマーが既にスタートしている場合は停止させる
    fido_lock_channel_timer_stop();

    // 直近レスポンスからの経過秒数監視を開始
    err_code = app_timer_start(m_lock_channel_timer_id, APP_TIMER_TICKS(lock_ms), NULL);
    if (err_code != NRF_SUCCESS) {
        fido_log_error("app_timer_start(m_lock_channel_timer_id) returns %d ", err_code);
    }
}
