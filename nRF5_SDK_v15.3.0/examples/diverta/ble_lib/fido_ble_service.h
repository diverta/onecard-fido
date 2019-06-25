/* 
 * File:   fido_ble_service.h
 * Author: makmorit
 *
 * Created on 2019/06/25, 13:30
 */
#ifndef FIDO_BLE_SERVICE_H
#define FIDO_BLE_SERVICE_H

#include "ble_advertising.h"
#include "ble_u2f.h"

#ifdef __cplusplus
extern "C" {
#endif

void       fido_ble_advertising_init(ble_advertising_init_t *p_init);
void       fido_ble_services_init(void);
ble_u2f_t *fido_ble_get_U2F_context(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BLE_SERVICE_H */
