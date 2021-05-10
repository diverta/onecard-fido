/* 
 * File:   app_bluetooth.h
 * Author: makmorit
 *
 * Created on 2021/04/06, 14:50
 */
#ifndef APP_BLUETOOTH_H
#define APP_BLUETOOTH_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool        app_ble_start_advertising(void);
bool        app_ble_stop_advertising(void);
void        app_bluetooth_start(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_BLUETOOTH_H */
