/* 
 * File:   fido_timer.c
 * Author: makmorit
 *
 * Created on 2019/06/18, 11:00
 */
#include "sdk_common.h"
#include "app_timer.h"

#include "fido_command.h"
#include "fido_log.h"

//
// 無通信タイマー
//
#define COMMUNICATION_INTERVAL_MSEC 10000
APP_TIMER_DEF(m_comm_interval_timer_id);

static void comm_interval_timeout_handler(void *p_context)
{
    // 直近のレスポンスから10秒を経過した場合、
    // FIDO機能処理タイムアウト時の処理を実行
    fido_command_on_process_timedout();
}

static ret_code_t comm_interval_timer_init(void)
{
    // 直近レスポンスからの経過秒数監視するためのタイマーを生成
    ret_code_t err_code;
    err_code = app_timer_create(&m_comm_interval_timer_id, APP_TIMER_MODE_SINGLE_SHOT, comm_interval_timeout_handler);
    if (err_code != NRF_SUCCESS) {
        fido_log_error("app_timer_create(m_comm_interval_timer_id) returns %d ", err_code);
    }
    
    return err_code;
}

void fido_comm_interval_timer_stop(void)
{
    // 直近レスポンスからの経過秒数監視を停止
    ret_code_t err_code = app_timer_stop(m_comm_interval_timer_id);
    if (err_code != NRF_SUCCESS) {
        fido_log_error("app_timer_stop(m_comm_interval_timer_id) returns %d ", err_code);
    }
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
    