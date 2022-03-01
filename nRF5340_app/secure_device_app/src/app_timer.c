/* 
 * File:   app_timer.c
 * Author: makmorit
 *
 * Created on 2021/04/07, 15:04
 */
#include <zephyr/types.h>
#include <zephyr.h>

#include "app_timer_define.h"

// ログ出力制御
#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_timer);

#define LOG_TIMER_START_STOP    false

//
// タイマー共通処理
//
static struct k_timer timer_for_longpush;
static struct k_timer timer_for_idling;
static struct k_timer timer_for_blinking;
static struct k_timer timer_for_ble_advertise;
static struct k_timer timer_for_generic_oneshot;
static struct k_timer timer_for_generic_repeat;

static void handler_for_longpush(struct k_timer *timer);
static void handler_for_idling(struct k_timer *timer);
static void handler_for_blinking(struct k_timer *timer);
static void handler_for_ble_advertise(struct k_timer *timer);
static void handler_for_generic_oneshot(struct k_timer *timer);
static void handler_for_generic_repeat(struct k_timer *timer);

void app_timer_initialize(void) 
{
    k_timer_init(&timer_for_longpush, handler_for_longpush, NULL);
    k_timer_init(&timer_for_idling,   handler_for_idling,   NULL);
    k_timer_init(&timer_for_blinking, handler_for_blinking, NULL);
    k_timer_init(&timer_for_ble_advertise,   handler_for_ble_advertise,   NULL);
    k_timer_init(&timer_for_generic_oneshot, handler_for_generic_oneshot, NULL);
    k_timer_init(&timer_for_generic_repeat,  handler_for_generic_repeat,  NULL);
}

static void app_timer_start(struct k_timer *timer, TIMER_CFG *cfg)
{
    k_timer_start(timer, K_MSEC(cfg->timeout_ms), K_NO_WAIT);
}

static void app_timer_stop(struct k_timer *timer)
{
    k_timer_stop(timer);
}

//
// ボタンが長押しされたことを検知するタイマー
//
TIMER_CFG cfg_longpush;

static void handler_for_longpush(struct k_timer *timer)
{
    // タイムアウト時の処理を実行する
    (void)timer;
    app_event_notify(cfg_longpush.callback_event);
}

void app_timer_start_for_longpush(uint32_t timeout_ms, APP_EVENT_T event)
{
    cfg_longpush.timeout_ms = timeout_ms;
    cfg_longpush.callback_event = event;
    cfg_longpush.is_repeat = false;
    app_timer_start(&timer_for_longpush, &cfg_longpush);

#if LOG_TIMER_START_STOP
    LOG_DBG("Set oneshot timer in %u msec", timeout_ms);
#endif
}

void app_timer_stop_for_longpush(void)
{
    app_timer_stop(&timer_for_longpush);

#if LOG_TIMER_START_STOP
    LOG_DBG("Canceled timer");
#endif
}

//
// アイドル状態が所定の時間連続したことを検知するタイマー
//
TIMER_CFG cfg_idling;

static void handler_for_idling(struct k_timer *timer)
{
    // タイムアウト時の処理を実行する
    (void)timer;
    app_event_notify(cfg_idling.callback_event);
}

void app_timer_start_for_idling(uint32_t timeout_ms, APP_EVENT_T event)
{
    cfg_idling.timeout_ms = timeout_ms;
    cfg_idling.callback_event = event;
    cfg_idling.is_repeat = false;
    app_timer_start(&timer_for_idling, &cfg_idling);

#if LOG_TIMER_START_STOP
    LOG_DBG("Set oneshot timer in %u msec", timeout_ms);
#endif
}

void app_timer_stop_for_idling(void)
{
    app_timer_stop(&timer_for_idling);

#if LOG_TIMER_START_STOP
    LOG_DBG("Canceled timer");
#endif
}

//
// LEDを点滅させるための繰り返しタイマー
//
TIMER_CFG cfg_blinking;

static void handler_for_blinking(struct k_timer *timer)
{
    // タイムアウト時の処理を実行する
    app_event_notify(cfg_blinking.callback_event);
    
    // 繰返しタイマーの場合は再度タイマーを開始
    if (cfg_blinking.is_repeat) {
        app_timer_start(timer, &cfg_blinking);
    }
}

void app_timer_start_for_blinking(uint32_t timeout_ms, APP_EVENT_T event)
{
    // APEVT_LED_BLINK時は、100 msごとに繰り返し
    cfg_blinking.timeout_ms = timeout_ms;
    cfg_blinking.callback_event = event;
    cfg_blinking.is_repeat = (event == APEVT_LED_BLINK);
    app_timer_start(&timer_for_blinking, &cfg_blinking);

#if LOG_TIMER_START_STOP
    LOG_DBG("Set repeat timer in %u msec", timeout_ms);
#endif
}

void app_timer_stop_for_blinking(void)
{
    app_timer_stop(&timer_for_blinking);

#if LOG_TIMER_START_STOP
    LOG_DBG("Canceled timer");
#endif
}

//
// BLEアドバタイズが所定の時間連続したことを検知するタイマー
//
TIMER_CFG cfg_ble_advertise;

static void handler_for_ble_advertise(struct k_timer *timer)
{
    // タイムアウト時の処理を実行する
    (void)timer;
    app_event_notify(cfg_ble_advertise.callback_event);
}

void app_timer_start_for_ble_advertise(uint32_t timeout_ms, APP_EVENT_T event)
{
    cfg_ble_advertise.timeout_ms = timeout_ms;
    cfg_ble_advertise.callback_event = event;
    cfg_ble_advertise.is_repeat = false;
    app_timer_start(&timer_for_ble_advertise, &cfg_ble_advertise);

#if LOG_TIMER_START_STOP
    LOG_DBG("Set repeat timer in %u msec", timeout_ms);
#endif
}

void app_timer_stop_for_ble_advertise(void)
{
    app_timer_stop(&timer_for_ble_advertise);

#if LOG_TIMER_START_STOP
    LOG_DBG("Canceled timer");
#endif
}

//
// 業務処理用 汎用ワンショットタイマー
//
TIMER_CFG cfg_generic_oneshot;
static void (*callback_for_generic_oneshot)(void);

static void handler_for_generic_oneshot(struct k_timer *timer)
{
    // タイムアウト時の処理を実行する
    (void)timer;
    if (callback_for_generic_oneshot != NULL) {
        (*callback_for_generic_oneshot)();
    }
}

void app_timer_start_for_generic_oneshot(uint32_t timeout_ms, void (*callback_func)(void))
{
    cfg_generic_oneshot.timeout_ms = timeout_ms;
    cfg_generic_oneshot.callback_event = APEVT_NONE;
    cfg_generic_oneshot.is_repeat = false;
    callback_for_generic_oneshot = callback_func;
    app_timer_start(&timer_for_generic_oneshot, &cfg_generic_oneshot);

#if LOG_TIMER_START_STOP
    LOG_DBG("Set repeat timer in %u msec", timeout_ms);
#endif
}

void app_timer_stop_for_generic_oneshot(void)
{
    callback_for_generic_oneshot = NULL;
    app_timer_stop(&timer_for_generic_oneshot);

#if LOG_TIMER_START_STOP
    LOG_DBG("Canceled timer");
#endif
}

//
// 業務処理用 汎用リピートタイマー
//
TIMER_CFG cfg_generic_repeat;
static void (*callback_for_generic_repeat)(void);

static void handler_for_generic_repeat(struct k_timer *timer)
{
    // タイムアウト時の処理を実行する
    if (callback_for_generic_repeat != NULL) {
        (*callback_for_generic_repeat)();
    }

    // 次のタイマーを実行
    app_timer_start(timer, &cfg_generic_repeat);
}

void app_timer_start_for_generic_repeat(uint32_t timeout_ms, void (*callback_func)(void))
{
    cfg_generic_repeat.timeout_ms = timeout_ms;
    cfg_generic_repeat.callback_event = APEVT_NONE;
    cfg_generic_repeat.is_repeat = true;
    callback_for_generic_repeat = callback_func;
    app_timer_start(&timer_for_generic_repeat, &cfg_generic_repeat);

#if LOG_TIMER_START_STOP
    LOG_DBG("Set repeat timer in %u msec", timeout_ms);
#endif
}

void app_timer_stop_for_generic_repeat(void)
{
    callback_for_generic_repeat = NULL;
    app_timer_stop(&timer_for_generic_repeat);

#if LOG_TIMER_START_STOP
    LOG_DBG("Canceled timer");
#endif
}
