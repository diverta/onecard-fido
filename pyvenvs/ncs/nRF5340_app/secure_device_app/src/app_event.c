/* 
 * File:   app_event.c
 * Author: makmorit
 *
 * Created on 2021/04/06, 15:13
 */
#include <zephyr/types.h>
#include <zephyr.h>

#include "app_event.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_event);

//
// イベント待ち行列の管理
//
K_FIFO_DEFINE(app_main_fifo);

typedef struct {
    void           *fifo_reserved;
    APP_EVENT_T     event;
} APP_MAIN_FIFO_T;

bool app_event_notify(APP_EVENT_T event)
{
    // 領域を確保
    size_t size = sizeof(APP_MAIN_FIFO_T);
    char *p_fifo = k_malloc(size);
    if (p_fifo == NULL) {
        LOG_ERR("APP_MAIN_FIFO_T allocation failed");
        return false;
    }

    // イベントデータを待ち行列にセット
    APP_MAIN_FIFO_T fifo = {
        .event = event
    };
    memcpy(p_fifo, &fifo, size);
    k_fifo_put(&app_main_fifo, p_fifo);
    return true;
}

//
// 各種イベント処理
//
static void button_pressed(APP_EVENT_T event)
{
    // ボタン検知時刻を取得
    static uint32_t time_pressed = 0;
    uint32_t time_now = app_board_kernel_uptime_ms_get();

    // ボタン検知間隔を取得
    uint32_t elapsed = time_now - time_pressed;
    time_pressed = time_now;

    // ボタン検知間隔で判定
    if (event == APEVT_BUTTON_RELEASED) {
        if (elapsed > 3000) {
            // 長押し
            LOG_DBG("Long pushed");
        } else {
            // 短押し
            LOG_DBG("Short pushed");
        }
    }
}

//
// 業務スレッドのイベント処理
//
void app_event_process(void)
{
    // イベント検知まで待機
    APP_MAIN_FIFO_T *fifo = k_fifo_get(&app_main_fifo, K_FOREVER);

    // FIFOデータを解放
    APP_EVENT_T event = fifo->event;
    k_free(fifo);

    // 仮の実装です。
    switch (event) {
        case APEVT_BUTTON_PUSHED:
        case APEVT_BUTTON_RELEASED:
            button_pressed(event);
            break;
        default:
            break;
    }
}
