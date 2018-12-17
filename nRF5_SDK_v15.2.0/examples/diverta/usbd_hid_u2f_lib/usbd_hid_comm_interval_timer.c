/* 
 * File:   usbd_hid_comm_interval_timer.c
 * Author: makmorit
 *
 * Created on 2018/11/26, 10:50
 */
#include "sdk_common.h"
#include "app_timer.h"

#include "hid_u2f_command.h"

// for logging informations
#define NRF_LOG_MODULE_NAME usbd_hid_comm_interval_timer
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define COMMUNICATION_INTERVAL_MSEC 10000

// 無通信タイマー
APP_TIMER_DEF(m_comm_interval_timer_id);
static bool app_timer_created = false;
static bool app_timer_started = false;

static void comm_interval_timeout_handler(void *p_context)
{
    // 直近のレスポンスから10秒を経過した場合、
    // U2F処理タイムアウト時の処理を実行
    hid_u2f_command_on_process_timedout();
}

static void comm_interval_timer_init(void)
{
    if (app_timer_created == true) {
        return;
    }

    // 直近レスポンスからの経過秒数監視するためのタイマーを生成
    uint32_t err_code;
    err_code = app_timer_create(&m_comm_interval_timer_id, APP_TIMER_MODE_SINGLE_SHOT, comm_interval_timeout_handler);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_create(m_comm_interval_timer_id) returns %d ", err_code);
        return;
    }
    app_timer_created = true;
}

void usbd_hid_comm_interval_timer_stop(void)
{
    if (app_timer_created == false || app_timer_started == false) {
        return;
    }

    // 直近レスポンスからの経過秒数監視を停止
    app_timer_started = false;
    uint32_t err_code = app_timer_stop(m_comm_interval_timer_id);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_stop(m_comm_interval_timer_id) returns %d ", err_code);
        return;
    }
}

void usbd_hid_comm_interval_timer_start(void)
{
    // タイマー生成
    if (app_timer_created == false) {
        comm_interval_timer_init();
        if (app_timer_created == false) {
            return;
        }
    }

    // タイマーが既にスタートしている場合は停止させる
    if (app_timer_started == true) {
        usbd_hid_comm_interval_timer_stop();
    }

    // 直近レスポンスからの経過秒数監視を開始
    uint32_t err_code = app_timer_start(m_comm_interval_timer_id, APP_TIMER_TICKS(COMMUNICATION_INTERVAL_MSEC), NULL);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_start(m_comm_interval_timer_id) returns %d ", err_code);
        return;
    }
    app_timer_started = true;
}
    