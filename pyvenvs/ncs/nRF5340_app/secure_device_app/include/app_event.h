/* 
 * File:   app_event.h
 * Author: makmorit
 *
 * Created on 2021/04/06, 15:13
 */
#ifndef APP_EVENT_H
#define APP_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

// イベント種別
typedef enum {
    APEVT_NONE = 0,
    APEVT_BUTTON_PUSHED,
    APEVT_BUTTON_PUSHED_LONG,
    APEVT_BUTTON_RELEASED,
    APEVT_BLE_CONNECTED,
    APEVT_BLE_DISCONNECTED,
    APEVT_IDLING_DETECTED,
} APP_EVENT_T;

//
// 関数群
//
bool    app_event_notify(APP_EVENT_T event);
void    app_event_process(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_EVENT_H */
