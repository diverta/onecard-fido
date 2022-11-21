/* 
 * File:   rtcc.c
 * Author: makmorit
 *
 * Created on 2022/11/16, 10:16
 */
#include <sdk_config.h>

#include "sdk_common.h"

// for logging informations
#define NRF_LOG_MODULE_NAME rtcc
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// RTCC関連
#include "rv3028c7_i2c.h"

// モジュール利用の可否を保持
static bool rtcc_is_available = false;

// 作業領域
static char work_buf[32];

void rtcc_init(void)
{
    // RTCCの初期化
    rtcc_is_available = rv3028c7_initialize();
    if (rtcc_is_available == false) {
        NRF_LOG_INFO("RTCC is unavailable");
        return;
    }
    if (rv3028c7_get_timestamp(work_buf, sizeof(work_buf)) == false) {
        return;
    }

    // 現在時刻を表示
    NRF_LOG_INFO("RTCC is available. Current timestamp: %s", work_buf);
}

bool rtcc_update_timestamp_by_unixtime(uint32_t unixtime, uint8_t timezone_diff_hours)
{
    // RTCCが搭載されていない場合は終了
    if (rtcc_is_available == false) {
        NRF_LOG_ERROR("RTCC is unavailable");
        return false;
    }

    // カウンターをRTCCに設定
    if (rv3028c7_set_timestamp(unixtime, timezone_diff_hours) == false) {
        return false;
    }

    // 設定された現在時刻を取得
    if (rv3028c7_get_timestamp(work_buf, sizeof(work_buf)) == false) {
        return false;
    }

    // 現在時刻を表示
    NRF_LOG_INFO("Current timestamp (updated by unixtime): %s", work_buf);
    return true;
}

bool rtcc_get_timestamp_string(char *buf, size_t size)
{
    // 設定された現在時刻を取得
    return rv3028c7_get_timestamp(buf, size);
}
