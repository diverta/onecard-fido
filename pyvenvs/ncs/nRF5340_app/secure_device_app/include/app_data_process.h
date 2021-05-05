/* 
 * File:   app_data_process.h
 * Author: makmorit
 *
 * Created on 2021/05/05, 15:37
 */
#ifndef APP_DATA_PROCESS_H
#define APP_DATA_PROCESS_H

#include "app_data_event.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        app_data_process_for_event(DATA_EVENT_T event, uint8_t *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* APP_DATA_PROCESS_H */
