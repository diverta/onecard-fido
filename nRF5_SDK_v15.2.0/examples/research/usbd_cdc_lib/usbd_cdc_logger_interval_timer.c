/* 
 * File:   usbd_cdc_logger_interval_timer.c
 * Author: makmorit
 *
 * Created on 2019/03/05, 10:50
 */
#include "sdk_common.h"
#include "app_timer.h"

// for logging informations
#define NRF_LOG_MODULE_NAME usbd_cdc_logger_interval_timer
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define COMMUNICATION_INTERVAL_MSEC 1000

// 無通信タイマー
APP_TIMER_DEF(m_interval_timer_id);
static bool app_timer_created = false;
static bool app_timer_started = false;

// 起動からの通算秒数
static uint32_t serial_sec = 0;

uint32_t usbd_cdc_logger_serial_second(void)
{
    return serial_sec;
}

static void interval_timeout_handle(void *p_context)
{
    // １秒ごとにログを出力する
    UNUSED_PARAMETER(p_context);
    ++serial_sec;

    // 最大秒数を超えたらゼロクリア
    // (60秒 * 60分 * 24時間 * 365日 = 31,536,000秒)
    if (serial_sec == 31536000) {
        serial_sec = 0;
    }
}

static void comm_interval_timer_init(void)
{
    if (app_timer_created == true) {
        return;
    }

    uint32_t err_code;
    err_code = app_timer_create(&m_interval_timer_id, APP_TIMER_MODE_REPEATED, interval_timeout_handle);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_create(m_interval_timer_id) returns %d ", err_code);
        return;
    }
    app_timer_created = true;
}

void usbd_cdc_logger_interval_timer_stop(void)
{
    if (app_timer_created == false || app_timer_started == false) {
        return;
    }

    app_timer_started = false;
    uint32_t err_code = app_timer_stop(m_interval_timer_id);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_stop(m_interval_timer_id) returns %d ", err_code);
        return;
    }
}

void usbd_cdc_logger_interval_timer_start(void)
{
    if (app_timer_created == false) {
        comm_interval_timer_init();
        if (app_timer_created == false) {
            return;
        }
    }

    if (app_timer_started == true) {
        usbd_cdc_logger_interval_timer_stop();
    }

    uint32_t err_code = app_timer_start(m_interval_timer_id, APP_TIMER_TICKS(COMMUNICATION_INTERVAL_MSEC), NULL);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_start(m_comm_interval_timer_id) returns %d ", err_code);
        return;
    }
    app_timer_started = true;
}
    