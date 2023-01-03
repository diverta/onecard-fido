/* 
 * File:   fido_timer_plat.c
 * Author: makmorit
 *
 * Created on 2019/06/18, 11:00
 */
#include "sdk_common.h"
#include "app_timer.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_timer_plat
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

//
// 無通信タイマー
//
APP_TIMER_DEF(m_comm_interval_timer_id);
static bool comm_interval_timer_created = false;
static void (*fido_comm_interval_timeout_handler)(void) = NULL;

static void comm_interval_timeout_handler(void *p_context)
{
    (void)p_context;
    (*fido_comm_interval_timeout_handler)();
}

static ret_code_t comm_interval_timer_init(void)
{
    if (comm_interval_timer_created) {
        return NRF_SUCCESS;
    }

    // 直近レスポンスからの経過秒数監視するためのタイマーを生成
    ret_code_t err_code = app_timer_create(&m_comm_interval_timer_id, APP_TIMER_MODE_SINGLE_SHOT, comm_interval_timeout_handler);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_create(m_comm_interval_timer_id) returns %d ", err_code);
    }
    
    comm_interval_timer_created = true;
    return err_code;
}

void fido_comm_interval_timer_stop(void)
{
    // 直近レスポンスからの経過秒数監視を停止
    app_timer_stop(m_comm_interval_timer_id);
}

void fido_comm_interval_timer_start(uint32_t timeout_msec, void (*_handler)(void))
{
    // タイマー生成
    fido_comm_interval_timeout_handler = _handler;
    ret_code_t err_code = comm_interval_timer_init();
    if (err_code != NRF_SUCCESS) {
        return;
    }

    // タイマーが既にスタートしている場合は停止させる
    fido_comm_interval_timer_stop();

    // 直近レスポンスからの経過秒数監視を開始
    err_code = app_timer_start(m_comm_interval_timer_id, APP_TIMER_TICKS(timeout_msec), NULL);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_start(m_comm_interval_timer_id) returns %d ", err_code);
    }
}

//
// LED点滅タイマー（処理中表示用）
//
APP_TIMER_DEF(m_led_on_off_timer_id);
static bool led_on_off_timer_created = false;
static void (*fido_processing_led_timedout_handler)(void) = NULL;

static void processing_led_timeout_handler(void *p_context)
{
    (void)p_context;
    (*fido_processing_led_timedout_handler)();
}

static ret_code_t processing_led_timer_init()
{
    if (led_on_off_timer_created) {
        return NRF_SUCCESS;
    }

    ret_code_t err_code = app_timer_create(&m_led_on_off_timer_id, APP_TIMER_MODE_REPEATED, processing_led_timeout_handler);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_create(m_led_on_off_timer_id) returns %d ", err_code);
    }
    
    led_on_off_timer_created = true;
    return err_code;
}

void fido_processing_led_timer_stop(void)
{
    // タイマーを停止する
    app_timer_stop(m_led_on_off_timer_id);
}

void fido_processing_led_timer_start(uint32_t on_off_interval_msec, void (*_handler)(void))
{
    // タイマー生成
    fido_processing_led_timedout_handler = _handler;
    ret_code_t err_code = processing_led_timer_init();
    if (err_code != NRF_SUCCESS) {
        return;
    }

    // タイマーが既にスタートしている場合は停止させる
    fido_processing_led_timer_stop();

    // タイマーを開始する
    err_code = app_timer_start(m_led_on_off_timer_id, APP_TIMER_TICKS(on_off_interval_msec), NULL);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_start(m_led_on_off_timer_id) returns %d ", err_code);
    }
}

//
// LED点滅タイマー（アイドル時表示用）
//
APP_TIMER_DEF(m_idling_led_timer_id);
static bool idling_led_timer_created = false;
static void (*fido_idling_led_timedout_handler)(void) = NULL;

static void idling_led_timeout_handler(void *p_context)
{
    (void)p_context;
    (*fido_idling_led_timedout_handler)();
}

static ret_code_t idling_led_timer_init()
{
    if (idling_led_timer_created) {
        return NRF_SUCCESS;
    }

    ret_code_t err_code = app_timer_create(&m_idling_led_timer_id, APP_TIMER_MODE_REPEATED, idling_led_timeout_handler);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_create(m_idling_led_timer_id) returns %d ", err_code);
    }
    
    idling_led_timer_created = true;
    return err_code;
}

void fido_idling_led_timer_stop(void)
{
    // タイマーを停止する
    app_timer_stop(m_idling_led_timer_id);
}

void fido_idling_led_timer_start(uint32_t on_off_interval_msec, void (*_handler)(void))
{
    // タイマー生成
    fido_idling_led_timedout_handler = _handler;
    ret_code_t err_code = idling_led_timer_init();
    if (err_code != NRF_SUCCESS) {
        return;
    }

    // タイマーが既にスタートしている場合は停止させる
    fido_idling_led_timer_stop();

    // タイマーを開始する
    err_code = app_timer_start(m_idling_led_timer_id, APP_TIMER_TICKS(on_off_interval_msec), NULL);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_start(m_idling_led_timer_id) returns %d ", err_code);
    }
}

//
// ボタン長押し検知用タイマー
//
APP_TIMER_DEF(m_long_push_timer_id);
static bool long_push_timer_created = false;
static void (*fido_button_long_push_timeout_handler)(void) = NULL;

static void button_long_push_timeout_handler(void *p_context)
{
    (void)p_context;
    (*fido_button_long_push_timeout_handler)();
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
        NRF_LOG_ERROR("app_timer_create(m_long_push_timer_id) returns %d ", err_code);
    }

    long_push_timer_created = true;
}

void fido_button_long_push_timer_stop(void)
{
    // タイマーを停止する
    app_timer_stop(m_long_push_timer_id);
}

void fido_button_long_push_timer_start(uint32_t timeout_msec, void (*_handler)(void))
{
    // タイマーを開始する
    fido_button_long_push_timeout_handler = _handler;
    ret_code_t err_code = app_timer_start(m_long_push_timer_id, APP_TIMER_TICKS(timeout_msec), NULL);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_start(m_long_push_timer_id) returns %d ", err_code);
    }
}
