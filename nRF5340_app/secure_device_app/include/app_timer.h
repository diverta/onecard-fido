/* 
 * File:   app_timer.h
 * Author: makmorit
 *
 * Created on 2021/04/07, 15:04
 */
#ifndef APP_TIMER_H
#define APP_TIMER_H

#include "app_event.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        app_timer_initialize(void);
void        app_timer_start_for_longpush(uint32_t timeout_ms, APP_EVENT_T event);
void        app_timer_stop_for_longpush(void);
void        app_timer_start_for_idling(uint32_t timeout_ms, APP_EVENT_T event);
void        app_timer_stop_for_idling(void);
void        app_timer_start_for_blinking_begin(uint32_t timeout_ms, APP_EVENT_T event);
void        app_timer_start_for_blinking(uint32_t timeout_ms, APP_EVENT_T event);
void        app_timer_stop_for_blinking(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_TIMER_H */
