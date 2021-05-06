/* 
 * File:   app_data_process.c
 * Author: makmorit
 *
 * Created on 2021/05/05, 15:37
 */
#include <zephyr/types.h>
#include <zephyr.h>

#include "app_data_event.h"
#include "app_custom.h"

// ログ出力制御
#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_data_process);

void app_data_process_for_event(DATA_EVENT_T event, uint8_t *data, size_t size)
{
    // イベントに対応する処理を実行
    switch (event) {
        case DATEVT_HID_REPORT_RECEIVED:
            app_custom_hid_report_received(data, size);
            break;
        case DATEVT_HID_REPORT_SENT:
            app_custom_hid_report_sent();
            break;
        default:
            break;
    }
}
