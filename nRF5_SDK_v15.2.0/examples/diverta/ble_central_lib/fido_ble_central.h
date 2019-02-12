/* 
 * File:   fido_ble_central.h
 * Author: makmorit
 *
 * Created on 2019/02/11, 16:59
 */
#ifndef FIDO_BLE_CENTRAL_H
#define FIDO_BLE_CENTRAL_H

#include "ble.h"

#ifdef __cplusplus
extern "C" {
#endif

void fido_ble_central_init(void);
void fido_ble_central_evt_handler(ble_evt_t const *p_ble_evt, void *p_context);
void fido_ble_central_scan_start(void);
void fido_ble_central_scan_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BLE_CENTRAL_H */
