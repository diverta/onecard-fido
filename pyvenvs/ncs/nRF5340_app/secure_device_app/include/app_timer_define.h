/* 
 * File:   app_timer_define.h
 * Author: makmorit
 *
 * Created on 2021/04/08, 16:49
 */
#ifndef APP_TIMER_DEFINE_H
#define APP_TIMER_DEFINE_H

#include "app_event.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// タイマーのチャネルID
// 
#define CHID_FOR_LONGPUSH           0
#define CHID_FOR_IDLING             1

//
// タイマーで使用する各種データを保持
//
typedef struct {
    uint8_t     chan_id;
    uint32_t    timeout_ms;
    APP_EVENT_T callback_event;
    bool        is_repeat;
    struct counter_alarm_cfg alarm_cfg;
} TIMER_CFG;

#ifdef __cplusplus
}
#endif

#endif /* APP_TIMER_DEFINE_H */
