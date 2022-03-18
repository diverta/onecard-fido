/* 
 * File:   app_event.c
 * Author: makmorit
 *
 * Created on 2021/04/06, 15:13
 */
#include <zephyr/types.h>
#include <zephyr.h>

#include "app_event.h"
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
K_FIFO_DEFINE(app_data_fifo);

typedef struct {
    void           *fifo_reserved;
    APP_EVENT_T     event;
} APP_MAIN_FIFO_T;

typedef struct {
    void           *fifo_reserved;
    DATA_EVENT_T    event;
    uint8_t         data[64];
    size_t          size;
} APP_DATA_FIFO_T;

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

bool app_event_notify_for_data(DATA_EVENT_T event, uint8_t *data, size_t data_size)
{
#if LOG_NOTIFIED_EVENT
    LOG_DBG("App data event notified (event type=%d)", event);
#endif

    // 領域を確保
    size_t size = sizeof(APP_DATA_FIFO_T);
    APP_DATA_FIFO_T *p_fifo = (APP_DATA_FIFO_T *)k_malloc(size);
    if (p_fifo == NULL) {
        LOG_ERR("APP_DATA_FIFO_T allocation failed");
        return false;
    }

    // イベントデータを待ち行列にセット
    p_fifo->event = event;
    p_fifo->size = data_size;
    memcpy(p_fifo->data, data, data_size);
    k_fifo_put(&app_data_fifo, p_fifo);
    return true;
}

//
// スレッドのイベント処理
//
static bool main_event_enabled = false;

void app_event_main_enable(bool b)
{
    main_event_enabled = b;
}

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
        // 各種イベントを処理
        if (main_event_enabled) {
            app_event_process();
        }
    }
}

//
// データ関連イベント処理
//
static bool data_event_enabled = false;

void app_event_data_enable(bool b)
{
    data_event_enabled = b;
}

static void app_data_event_process(void)
{
    // イベント検知まで待機
    APP_DATA_FIFO_T *fifo = k_fifo_get(&app_data_fifo, K_FOREVER);

    // イベントに対応する処理を実行
    if (data_event_enabled) {
        app_process_for_data_event(fifo->event, fifo->data, fifo->size);
    }

    // FIFOデータを解放
    k_free(fifo);
}

static void app_data_thread(void)
{
    while (true) {
        // 各種イベントを処理
        if (main_event_enabled) {
            app_data_event_process();
        }
    }
}

//
// スレッド本体
//
// STACKSIZE: size of stack area used by thread
// PRIORITY:  scheduling priority used by thread
#define STACKSIZE   8192
#define PRIORITY    7
K_THREAD_DEFINE(app_main_thread_id, STACKSIZE, app_main_thread, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(app_data_thread_id, STACKSIZE, app_data_thread, NULL, NULL, NULL, PRIORITY, 0, 0);
