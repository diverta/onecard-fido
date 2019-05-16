/* 
 * File:   fido_ble_central.h
 * Author: makmorit
 *
 * Created on 2019/02/11, 16:59
 */
#ifndef FIDO_BLE_CENTRAL_H
#define FIDO_BLE_CENTRAL_H

#include "ble.h"
#include "nrf_ble_gatt.h"

#ifdef __cplusplus
extern "C" {
#endif

void   fido_ble_central_init(void);
void   fido_ble_central_evt_handler(ble_evt_t const *p_ble_evt, void *p_context);
void   fido_ble_central_gatt_evt_handler(nrf_ble_gatt_t *p_gatt, nrf_ble_gatt_evt_t const *p_evt);
void   fido_ble_central_scan_start(void);
void   fido_ble_central_scan_stop(void);
size_t get_adv_stat_info_string(uint32_t serial_num, char *adv_stat_info_string);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BLE_CENTRAL_H */
