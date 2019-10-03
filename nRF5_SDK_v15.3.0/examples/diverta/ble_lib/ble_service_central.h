/* 
 * File:   ble_service_central.h
 * Author: makmorit
 *
 * Created on 2019/10/02, 14:47
 */
#ifndef BLE_SERVICE_CENTRAL_H
#define BLE_SERVICE_CENTRAL_H

#include "ble.h"

#ifdef __cplusplus
extern "C" {
#endif

void ble_service_central_init(void);
void ble_service_central_scan_start(void);
void ble_service_central_scan_stop(void);
void ble_service_central_gap_adv_report(ble_evt_t const *p_ble_evt);

#ifdef __cplusplus
}
#endif

#endif /* BLE_SERVICE_CENTRAL_H */
