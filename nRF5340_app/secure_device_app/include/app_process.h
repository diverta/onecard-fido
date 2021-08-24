/* 
 * File:   app_process.h
 * Author: makmorit
 *
 * Created on 2021/04/28, 10:22
 */
#ifndef APP_PROCESS_H
#define APP_PROCESS_H

#include "app_event.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        app_process_for_event(APP_EVENT_T event);
void        app_process_for_data_event(DATA_EVENT_T event, uint8_t *data, size_t size);
void        app_process_initialize(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_PROCESS_H */
