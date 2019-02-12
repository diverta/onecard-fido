/* 
 * File:   fido_ble_peripheral.h
 * Author: makmorit
 *
 * Created on 2019/02/11, 15:04
 */
#ifndef FIDO_BLE_PERIPHERAL_H
#define FIDO_BLE_PERIPHERAL_H

#include "ble.h"

#ifdef __cplusplus
extern "C" {
#endif

void fido_ble_peripheral_init(void);
void fido_ble_peripheral_evt_handler(ble_evt_t const *p_ble_evt, void *p_context);
void fido_ble_peripheral_advertising_start(void);
void fido_ble_peripheral_advertising_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BLE_PERIPHERAL_H */

