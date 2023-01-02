/* 
 * File:   fido_ble_service.h
 * Author: makmorit
 *
 * Created on 2019/06/25, 13:30
 */
#ifndef FIDO_BLE_SERVICE_H
#define FIDO_BLE_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void       fido_ble_advertising_init(void *p_init);
void       fido_ble_services_init(void);
void      *fido_ble_get_U2F_context(void);
void       fido_ble_service_disconnect_force(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BLE_SERVICE_H */
