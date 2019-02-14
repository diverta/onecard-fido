/* 
 * File:   fido_ble_central_nus.h
 * Author: makmorit
 *
 * Created on 2019/02/14, 11:19
 */

#ifndef FIDO_BLE_CENTRAL_NUS_H
#define FIDO_BLE_CENTRAL_NUS_H

#include "ble.h"
#include "ble_db_discovery.h"

#ifdef __cplusplus
extern "C" {
#endif

void fido_ble_central_nus_init(void);
void fido_ble_central_nus_on_db_disc_evt(ble_db_discovery_evt_t *p_evt);
void fido_ble_central_nus_evt_connected(ble_evt_t *p_ble_evt, void *p_context);

ble_uuid_t const *fido_ble_central_nus_uuid(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BLE_CENTRAL_NUS_H */
