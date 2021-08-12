/* 
 * File:   app_ble_smp.h
 * Author: makmorit
 *
 * Created on 2021/08/09, 14:59
 */
#ifndef APP_BLE_SMP_H
#define APP_BLE_SMP_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool        app_ble_smp_ad_uuid_set(void *data);
void        app_ble_smp_register_group(void);
void        app_ble_smp_bt_register(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_BLE_SMP_H */
