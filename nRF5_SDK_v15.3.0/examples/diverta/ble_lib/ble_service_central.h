/* 
 * File:   ble_service_central.h
 * Author: makmorit
 *
 * Created on 2019/10/02, 14:47
 */
#ifndef BLE_SERVICE_CENTRAL_H
#define BLE_SERVICE_CENTRAL_H

#include "ble.h"
#include "ble_gap.h"

#ifdef __cplusplus
extern "C" {
#endif

void ble_service_central_init(void);
void ble_service_central_scan_start(uint32_t timeout_msec, void (*_resume_function)(void));
void ble_service_central_scan_stop(void);
bool ble_service_central_request_connection(ble_gap_addr_t *p_addr, void (*_resume_function)(void));
bool ble_service_central_request_disconnection(void);
void ble_service_central_gap_connected(ble_evt_t const *p_ble_evt);
void ble_service_central_gap_disconnected(ble_evt_t const *p_ble_evt);
void ble_service_central_gap_adv_report(ble_evt_t const *p_ble_evt);

#ifdef __cplusplus
}
#endif

#endif /* BLE_SERVICE_CENTRAL_H */
