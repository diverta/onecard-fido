/* 
 * File:   app_event.h
 * Author: makmorit
 *
 * Created on 2021/04/06, 15:13
 */
#ifndef APP_EVENT_H
#define APP_EVENT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// イベント種別
typedef enum {
    APEVT_NONE = 0,
    APEVT_BUTTON_PUSHED,
    APEVT_BUTTON_PUSHED_LONG,
    APEVT_BUTTON_RELEASED,
    APEVT_BUTTON_1_PUSHED,
    APEVT_BUTTON_1_RELEASED,
    APEVT_USB_CONNECTED,
    APEVT_USB_CONFIGURED,
    APEVT_USB_DISCONNECTED,
    APEVT_BLE_AVAILABLE,
    APEVT_BLE_UNAVAILABLE,
    APEVT_BLE_ADVERTISE_STARTED,
    APEVT_BLE_ADVERTISE_STOPPED,
    APEVT_BLE_CONNECTED,
    APEVT_BLE_DISCONNECTED,
    APEVT_IDLING_DETECTED,
    APEVT_ENTER_TO_BOOTLOADER,
    APEVT_LED_BLINK_BEGIN,
    APEVT_LED_BLINK,
} APP_EVENT_T;

// データ関連イベント種別
typedef enum {
    DATEVT_NONE = 0,
    DATEVT_HID_DATA_FRAME_RECEIVED,
    DATEVT_HID_REPORT_SENT,
    DATEVT_CCID_DATA_FRAME_RECEIVED,
    DATEVT_BLE_DATA_FRAME_RECEIVED,
    DATEVT_BLE_RESPONSE_SENT,
} DATA_EVENT_T;

//
// 関数群
//
bool        app_event_notify(APP_EVENT_T event);
bool        app_event_notify_for_data(DATA_EVENT_T event, uint8_t *data, size_t data_size);
void        app_event_main_enable(bool b);
void        app_event_data_enable(bool b);

#ifdef __cplusplus
}
#endif

#endif /* APP_EVENT_H */
