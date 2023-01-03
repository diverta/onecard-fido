/* 
 * File:   fido_ble_event.h
 * Author: makmorit
 *
 * Created on 2018/10/09, 11:59
 */

#ifndef FIDO_BLE_EVENT_H
#define FIDO_BLE_EVENT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool fido_ble_evt_handler(void const *ble_evt, void *p_context);
bool fido_ble_pm_evt_handler(void const *pm_evt);
void fido_ble_sleep_mode_enter(void);
void fido_ble_on_process_timedout(void);
void fido_ble_do_process(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BLE_EVENT_H */

