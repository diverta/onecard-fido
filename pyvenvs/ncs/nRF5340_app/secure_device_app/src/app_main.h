/* 
 * File:   app_main.h
 * Author: makmorit
 *
 * Created on 2021/04/02, 15:04
 */
#ifndef APP_MAIN_H
#define APP_MAIN_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// イベント種別
typedef enum {
    APEVT_NONE = 0,
    APEVT_BUTTON_PUSHED,
    APEVT_BUTTON_RELEASED,
} APP_EVENT_T;

//
// 関数群
//
void    app_main_init(void);
bool    app_main_initialized(void);
bool    app_main_event_set(APP_EVENT_T event);
void    app_bluetooth_start(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_MAIN_H */
