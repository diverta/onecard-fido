/* 
 * File:   app_timer.c
 * Author: makmorit
 *
 * Created on 2021/04/07, 15:04
 */
#include <zephyr/types.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/counter.h>
#include <errno.h>

#include "app_timer.h"
#include "app_timer_define.h"

// ログ出力制御
#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_timer);

#define LOG_TIMER_START_STOP    false

//
// タイマー共通処理
//
#define TIMER DT_LABEL(DT_NODELABEL(rtc0))

static const struct device *m_counter_dev;
static void counter_interrupted(const struct device *counter_dev, uint8_t chan_id, uint32_t ticks, void *user_data);

void app_timer_initialize(void) 
{
    m_counter_dev = device_get_binding(TIMER);
    if (m_counter_dev != NULL) {
        counter_start(m_counter_dev);
        LOG_INF("Alarm counter for timer started (timer=%s)", TIMER);

    } else {
        LOG_ERR("Timer (%s) device not found", TIMER);
    }
}

static void app_timer_start(uint8_t chan_id, TIMER_CFG *cfg)
{
    cfg->alarm_cfg.flags = 0;
    cfg->alarm_cfg.ticks = counter_us_to_ticks(m_counter_dev, cfg->timeout_ms * 1000);
    cfg->alarm_cfg.callback = counter_interrupted;
    cfg->alarm_cfg.user_data = cfg;

    int err = counter_set_channel_alarm(m_counter_dev, chan_id, &cfg->alarm_cfg);
    if (-EINVAL == err) {
        LOG_ERR("Alarm settings invalid (chan_id=%d)", chan_id);
    } else if (-ENOTSUP == err) {
        LOG_ERR("Alarm setting request not supported (chan_id=%d)", chan_id);
    } else if (err != 0) {
        LOG_ERR("Error counter_set_channel_alarm returns %d (chan_id=%d)", err, chan_id);
    }

#if LOG_TIMER_START_STOP
    LOG_DBG("Set alarm in %u msec (chan_id=%d)",
            (uint32_t)(counter_ticks_to_us(m_counter_dev, cfg->alarm_cfg.ticks) / USEC_PER_MSEC), chan_id);
#endif
}

static void app_timer_stop(uint8_t chan_id)
{
    counter_cancel_channel_alarm(m_counter_dev, chan_id);

#if LOG_TIMER_START_STOP
    LOG_DBG("Canceled alarm (chan_id=%d)", chan_id);
#endif
}

//
// タイムアウト時の処理
//
static void counter_interrupted(const struct device *counter_dev, uint8_t chan_id, uint32_t ticks, void *user_data)
{
#if LOG_TIMER_START_STOP
    LOG_DBG("Alarm by counter interrupt (chan_id=%d)", chan_id);
#endif

    // 業務処理スレッドにイベントを引き渡す
    TIMER_CFG *cfg = (TIMER_CFG *)user_data;
    app_event_notify(cfg->callback_event);
    
    // 繰返しタイマーの場合は再度タイマーを開始
    if (cfg->is_repeat) {
        app_timer_start(chan_id, user_data);
    }
}

//
// ボタンが長押しされたことを検知するタイマー
//
TIMER_CFG cfg_longpush;

void app_timer_start_for_longpush(uint32_t timeout_ms, APP_EVENT_T event)
{
    cfg_longpush.timeout_ms = timeout_ms;
    cfg_longpush.callback_event = event;
    cfg_longpush.is_repeat = false;
    app_timer_start(CHID_FOR_LONGPUSH, &cfg_longpush);
}

void app_timer_stop_for_longpush(void)
{
    app_timer_stop(CHID_FOR_LONGPUSH);
}

//
// アイドル状態が所定の時間連続したことを検知するタイマー
//
TIMER_CFG cfg_idling;

void app_timer_start_for_idling(uint32_t timeout_ms, APP_EVENT_T event)
{
    cfg_idling.timeout_ms = timeout_ms;
    cfg_idling.callback_event = event;
    cfg_idling.is_repeat = false;
    app_timer_start(CHID_FOR_IDLING, &cfg_idling);
}

void app_timer_stop_for_idling(void)
{
    app_timer_stop(CHID_FOR_IDLING);
}

//
// LEDを点滅させるための繰り返しタイマー
//
TIMER_CFG cfg_blinking;

void app_timer_start_for_blinking(void)
{
    // 100 msごとに繰り返し
    cfg_blinking.timeout_ms = 100;
    cfg_blinking.callback_event = APEVT_LED_BLINK;
    cfg_blinking.is_repeat = true;
    app_timer_start(CHID_FOR_BLINKING, &cfg_blinking);
}

void app_timer_stop_for_blinking(void)
{
    app_timer_stop(CHID_FOR_BLINKING);
}
