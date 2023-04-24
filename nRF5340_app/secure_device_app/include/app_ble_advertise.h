/* 
 * File:   app_ble_advertise.h
 * Author: makmorit
 *
 * Created on 2023/04/24, 11:05
 */
#ifndef APP_BLE_ADVERTISE_H
#define APP_BLE_ADVERTISE_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        app_ble_advertise_init(void);
void        app_ble_advertise_start(void);
void        app_ble_advertise_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_BLE_ADVERTISE_H */
