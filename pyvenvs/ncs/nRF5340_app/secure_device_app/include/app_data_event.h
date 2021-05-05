/* 
 * File:   app_data_event.h
 * Author: makmorit
 *
 * Created on 2021/05/05, 15:37
 */
#ifndef APP_DATA_EVENT_H
#define APP_DATA_EVENT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// イベント種別
typedef enum {
    DATEVT_NONE = 0,
    DATEVT_HID_REPORT_RECEIVED,
} DATA_EVENT_T;

//
// 関数群
//
bool        app_data_event_notify(DATA_EVENT_T event, uint8_t *data, size_t data_size);

#ifdef __cplusplus
}
#endif

#endif /* APP_DATA_EVENT_H */
