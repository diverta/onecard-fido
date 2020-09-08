/* 
 * File:   rtc2.h
 * Author: makmorit
 *
 * Created on 2020/09/07, 15:49
 */
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrfx_rtc.h"
#include "app_error.h"

// for logging informations
#define NRF_LOG_MODULE_NAME rtc2
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for debug
#define LOG_DEBUG_SECONDS false

static const nrfx_rtc_t rtc = NRFX_RTC_INSTANCE(2);
static uint32_t m_rtc_seconds = 0;

static void rtc2_handler(nrfx_rtc_int_type_t int_type)
{
    if (int_type == NRFX_RTC_INT_COMPARE3) {
        m_rtc_seconds++;
        nrfx_rtc_counter_clear(&rtc);
        nrfx_rtc_int_enable(&rtc, RTC_CHANNEL_INT_MASK(NRFX_RTC_INT_COMPARE3));
#if LOG_DEBUG_SECONDS
        NRF_LOG_DEBUG("App running %d seconds", m_rtc_seconds);
#endif
    }
}

void rtc2_init(void)
{
    // RTC2初期化
    nrfx_rtc_config_t rtc_config = NRFX_RTC_DEFAULT_CONFIG;
    ret_code_t err_code = nrfx_rtc_init(&rtc, &rtc_config, rtc2_handler);
    APP_ERROR_CHECK(err_code);

    // 1秒ごとに割込み発生させる設定
    nrfx_rtc_tick_enable(&rtc, false);
    err_code = nrfx_rtc_cc_set(&rtc, NRFX_RTC_INT_COMPARE3, RTC_INPUT_FREQ, true);
    APP_ERROR_CHECK(err_code);

    // RTCを始動
    nrfx_rtc_enable(&rtc);
    NRF_LOG_DEBUG("rtc2_init() done");
}

uint32_t rtc2_seconds_get(void)
{
    return m_rtc_seconds;
}
