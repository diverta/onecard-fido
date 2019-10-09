/* 
 * File:   ble_service_peripheral.h
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

bool fido_ble_peripheral_mode(void);
void fido_ble_peripheral_init(void);
void fido_ble_peripheral_advertising_start(void);
void fido_ble_peripheral_start(void);
void fido_ble_peripheral_timer_start(void);

void ble_peripheral_gap_connected(ble_evt_t const *p_ble_evt);
void ble_peripheral_gap_disconnected(ble_evt_t const *p_ble_evt);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BLE_PERIPHERAL_H */
