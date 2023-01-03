/* 
 * File:   ble_service_peripheral.h
 * Author: makmorit
 *
 * Created on 2019/02/11, 15:04
 */
#ifndef BLE_SERVICE_PERIPHERAL_H
#define BLE_SERVICE_PERIPHERAL_H

#ifdef __cplusplus
extern "C" {
#endif

void ble_service_peripheral_init(void);
void ble_service_peripheral_advertising_start(void);
void ble_service_peripheral_advertising_stop(void);
void ble_service_peripheral_start(void);
void ble_service_peripheral_timer_start(void);

void ble_service_peripheral_gap_connected(void const *p_evt);
void ble_service_peripheral_gap_disconnected(void const *p_evt);

#ifdef __cplusplus
}
#endif

#endif /* BLE_SERVICE_PERIPHERAL_H */
