/* 
 * File:   ble_service_common.h
 * Author: makmorit
 *
 * Created on 2019/10/02, 13:32
 */
#ifndef BLE_SERVICE_COMMON_H
#define BLE_SERVICE_COMMON_H

#include "ble.h"
#include "nrf_ble_gatt.h"

#ifdef __cplusplus
extern "C" {
#endif

bool ble_service_peripheral_mode(void);
void ble_service_peripheral_mode_set(bool b);

void ble_service_common_evt_handler(ble_evt_t const *p_ble_evt, void *p_context);
void ble_service_common_gatt_evt_handler(nrf_ble_gatt_t *p_gatt, nrf_ble_gatt_evt_t const *p_evt);
void ble_service_common_init(void);
void ble_service_common_disable_peripheral(void);
bool ble_service_common_erase_bond_data(void (*_response_func)(bool));

#ifdef __cplusplus
}
#endif

#endif /* BLE_SERVICE_COMMON_H */
