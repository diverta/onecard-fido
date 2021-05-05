/* 
 * File:   app_event.c
 * Author: makmorit
 *
 * Created on 2021/04/06, 15:13
 */
#include <zephyr/types.h>
#include <zephyr.h>

#include "app_event.h"
#include "app_main.h"
#include "app_process.h"

// ログ出力制御
#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_event);

#define LOG_NOTIFIED_EVENT      false

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
#if LOG_NOTIFIED_EVENT
    LOG_DBG("App event notified (event type=%d)", event);
#endif

    // 領域を確保
    size_t size = sizeof(APP_MAIN_FIFO_T);
    APP_MAIN_FIFO_T *p_fifo = (APP_MAIN_FIFO_T *)k_malloc(size);
    if (p_fifo == NULL) {
        LOG_ERR("APP_MAIN_FIFO_T allocation failed");
        return false;
    }

    // イベントデータを待ち行列にセット
    p_fifo->event = event;
    k_fifo_put(&app_main_fifo, p_fifo);
    return true;
}

//
// スレッドのイベント処理
//
static void app_event_process(void)
{
    // イベント検知まで待機
    APP_MAIN_FIFO_T *fifo = k_fifo_get(&app_main_fifo, K_FOREVER);

    // イベントに対応する処理を実行
    app_process_for_event(fifo->event);

    // FIFOデータを解放
    k_free(fifo);
}

static void app_main_thread(void)
{
    while (true) {
        // アプリケーション初期化前の場合はイベントを無視
        if (app_main_initialized() == false) {
            continue;
        }

        // 各種イベントを処理
        app_event_process();
    }
}

// STACKSIZE: size of stack area used by thread
// PRIORITY:  scheduling priority used by thread
#define STACKSIZE   1024
#define PRIORITY    7
K_THREAD_DEFINE(app_main_thread_id, STACKSIZE, app_main_thread, NULL, NULL, NULL, PRIORITY, 0, 0);
