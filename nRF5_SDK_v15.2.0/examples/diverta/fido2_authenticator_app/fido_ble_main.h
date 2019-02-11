/* 
 * File:   fido_ble_main.h
 * Author: makmorit
 *
 * Created on 2018/10/08, 10:37
 */

#ifndef FIDO_BLE_MAIN_H
#define FIDO_BLE_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_ble_gatt.h"
#include "ble_advertising.h"
#include "ble_u2f.h"

void fido_button_timers_init(void);
void fido_button_init(void);
void fido_ble_advertising_init(ble_advertising_init_t *p_init);
void fido_ble_services_init(void);
void fido_ble_peer_manager_init(void);
ble_u2f_t *fido_ble_get_U2F_context(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BLE_MAIN_H */

