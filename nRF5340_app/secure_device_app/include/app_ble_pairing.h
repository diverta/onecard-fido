/* 
 * File:   app_ble_pairing.h
 * Author: makmorit
 *
 * Created on 2021/04/27, 10:18
 */
#ifndef APP_BLE_PAIRING_H
#define APP_BLE_PAIRING_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool        app_ble_pairing_mode_set(bool b);
bool        app_ble_pairing_mode(void);
bool        app_ble_pairing_erase_bond_data(void (*response_func)(bool));

#ifdef __cplusplus
}
#endif

#endif /* APP_BLE_PAIRING_H */
