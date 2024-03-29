/* 
 * File:   ble_service_common.h
 * Author: makmorit
 *
 * Created on 2019/10/02, 13:32
 */
#ifndef BLE_SERVICE_COMMON_H
#define BLE_SERVICE_COMMON_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool ble_service_peripheral_mode(void);
void ble_service_peripheral_mode_set(bool b);
bool ble_service_peripheral_is_busy(void);
void ble_service_peripheral_set_busy(bool b);
bool ble_service_peripheral_mainsw_event_handler(void);

void ble_service_common_init(void);
void ble_service_common_disable_peripheral(void);

#ifdef __cplusplus
}
#endif

#endif /* BLE_SERVICE_COMMON_H */
