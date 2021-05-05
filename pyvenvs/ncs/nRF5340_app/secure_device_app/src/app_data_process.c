/* 
 * File:   app_data_process.c
 * Author: makmorit
 *
 * Created on 2021/05/05, 15:37
 */
#include <zephyr/types.h>
#include <zephyr.h>

#include "app_data_event.h"

// ログ出力制御
#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_data_process);

#define LOG_DEBUG_HID_REPORT        false

//
// データ処理イベント関連
//
static void hid_report_received(uint8_t *data, size_t size)
{
#if LOG_DEBUG_HID_REPORT
    LOG_DBG("received %d bytes", size);
    LOG_HEXDUMP_DBG(data, size, "HID report");
#endif
}

void app_data_process_for_event(DATA_EVENT_T event, uint8_t *data, size_t size)
{
    // イベントに対応する処理を実行
    switch (event) {
        case DATEVT_HID_REPORT_RECEIVED:
            hid_report_received(data, size);
            break;
        default:
            break;
    }
}
