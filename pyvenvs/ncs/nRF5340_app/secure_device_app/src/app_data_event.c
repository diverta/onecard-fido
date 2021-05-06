/* 
 * File:   app_data_event.c
 * Author: makmorit
 *
 * Created on 2021/05/05, 15:37
 */
#include <zephyr/types.h>
#include <zephyr.h>

#include "app_data_event.h"
#include "app_process.h"
#include "app_main.h"

// ログ出力制御
#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_data_event);

#define LOG_NOTIFIED_EVENT      false

//
// データ処理スレッド関連
//
K_FIFO_DEFINE(app_data_fifo);

typedef struct {
    void           *fifo_reserved;
    DATA_EVENT_T    event;
    uint8_t         data[64];
    size_t          size;
} APP_DATA_FIFO_T;

bool app_data_event_notify(DATA_EVENT_T event, uint8_t *data, size_t data_size)
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

static void event_process(void)
{
    // イベント検知まで待機
    APP_DATA_FIFO_T *fifo = k_fifo_get(&app_data_fifo, K_FOREVER);

    // イベントに対応する処理を実行
    app_process_for_data_event(fifo->event, fifo->data, fifo->size);

    // FIFOデータを解放
    k_free(fifo);
}

static void app_data_thread(void)
{
    while (true) {
        // アプリケーション初期化前の場合はイベントを無視
        if (app_main_initialized() == false) {
            continue;
        }

        // 各種イベントを処理
        event_process();
    }
}

// STACKSIZE: size of stack area used by thread
// PRIORITY:  scheduling priority used by thread
#define STACKSIZE   1024
#define PRIORITY    7
K_THREAD_DEFINE(app_data_thread_id, STACKSIZE, app_data_thread, NULL, NULL, NULL, PRIORITY, 0, 0);
