/* 
 * File:   ble_service_common.h
 * Author: makmorit
 *
 * Created on 2019/10/02, 13:32
 */
#ifndef BLE_SERVICE_COMMON_H
#define BLE_SERVICE_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

void ble_service_common_evt_handler(ble_evt_t const *p_ble_evt, void *p_context);
void ble_service_common_gatt_evt_handler(nrf_ble_gatt_t *p_gatt, nrf_ble_gatt_evt_t const *p_evt);

#ifdef __cplusplus
}
#endif

#endif /* BLE_SERVICE_COMMON_H */
