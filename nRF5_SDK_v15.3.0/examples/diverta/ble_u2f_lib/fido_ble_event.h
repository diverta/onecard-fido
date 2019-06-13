/* 
 * File:   fido_ble_event.h
 * Author: makmorit
 *
 * Created on 2018/10/09, 11:59
 */

#ifndef FIDO_BLE_EVENT_H
#define FIDO_BLE_EVENT_H

#include "ble.h"
#include "peer_manager_types.h"

#ifdef __cplusplus
extern "C" {
#endif

bool fido_ble_evt_handler(ble_evt_t const *p_ble_evt, void *p_context);
bool fido_ble_pm_evt_handler(pm_evt_t const *p_evt);
void fido_ble_sleep_mode_enter(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BLE_EVENT_H */

