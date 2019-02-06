/* 
 * File:   fido_lock_channel.c
 * Author: makmorit
 *
 * Created on 2019/02/06, 13:28
 */
#include "sdk_common.h"
#include "app_timer.h"

#include "hid_fido_command.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_lock_channel
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// ロック対象CIDを保持
static uint32_t cid_for_lock;

// ロックタイマー
APP_TIMER_DEF(m_lock_channel_timer_id);
static bool app_timer_created = false;
static bool app_timer_started = false;

static void lock_channel_timeout_handler(void *p_context)
{
    // 所定の秒数を経過した場合、
    // ロック対象CIDをクリア
    cid_for_lock = 0;
    NRF_LOG_INFO("Lock timed out");
}

static void lock_channel_timer_init(void)
{
    if (app_timer_created == true) {
        return;
    }

    // 直近レスポンスからの経過秒数監視するためのタイマーを生成
    uint32_t err_code;
    err_code = app_timer_create(&m_lock_channel_timer_id, APP_TIMER_MODE_SINGLE_SHOT, lock_channel_timeout_handler);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_create(m_lock_channel_timer_id) returns %d ", err_code);
        return;
    }
    app_timer_created = true;
}

static void fido_lock_channel_timer_stop(void)
{
    if (app_timer_created == false || app_timer_started == false) {
        return;
    }

    // 直近レスポンスからの経過秒数監視を停止
    app_timer_started = false;
    uint32_t err_code = app_timer_stop(m_lock_channel_timer_id);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_stop(m_lock_channel_timer_id) returns %d ", err_code);
        return;
    }
}

static void fido_lock_channel_timer_start(uint32_t lock_ms)
{
    // タイマー生成
    if (app_timer_created == false) {
        lock_channel_timer_init();
        if (app_timer_created == false) {
            return;
        }
    }

    // タイマーが既にスタートしている場合は停止させる
    if (app_timer_started == true) {
        fido_lock_channel_timer_stop();
    }

    // 直近レスポンスからの経過秒数監視を開始
    uint32_t err_code = app_timer_start(m_lock_channel_timer_id, APP_TIMER_TICKS(lock_ms), NULL);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_start(m_lock_channel_timer_id) returns %d ", err_code);
        return;
    }
    app_timer_started = true;
}

void fido_lock_channel_start(uint32_t cid, uint8_t lock_param)
{
    // ロックタイマーは最大10秒とする
    if (lock_param > 10) {
        lock_param = 10;
    }
    
    // ロックタイマーを開始
    uint32_t lock_ms = (uint32_t)lock_param * 1000;
    fido_lock_channel_timer_start(lock_ms);

    // パラメーターが指定されていた場合
    // ロック対象CIDを設定
    cid_for_lock = cid;
    NRF_LOG_INFO("Lock command done: CID(0x%08x) parameter(%d) ", cid, lock_param);
}

uint32_t fido_lock_channel_cid(void)
{
    // 現在ロック対象となっているCIDを戻す
    return cid_for_lock;
}

void fido_lock_channel_cancel(void)
{
    // ロック対象CIDをクリア
    cid_for_lock = 0;
    NRF_LOG_INFO("Unlock command done ");

    // ロックタイマーを停止
    fido_lock_channel_timer_stop();
}
