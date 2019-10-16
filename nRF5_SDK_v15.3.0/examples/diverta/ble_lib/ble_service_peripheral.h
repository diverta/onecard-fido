/* 
 * File:   ble_service_peripheral.h
 * Author: makmorit
 *
 * Created on 2019/02/11, 15:04
 */
#ifndef BLE_SERVICE_PERIPHERAL_H
#define BLE_SERVICE_PERIPHERAL_H

#include "ble.h"

#ifdef __cplusplus
extern "C" {
#endif

bool ble_service_peripheral_mode(void);
void ble_service_peripheral_init(void);
void ble_service_peripheral_advertising_start(void);
void ble_service_peripheral_start(void);
void ble_service_peripheral_timer_start(void);

void ble_service_peripheral_gap_connected(ble_evt_t const *p_ble_evt);
void ble_service_peripheral_gap_disconnected(ble_evt_t const *p_ble_evt);

#ifdef __cplusplus
}
#endif

#endif /* BLE_SERVICE_PERIPHERAL_H */
