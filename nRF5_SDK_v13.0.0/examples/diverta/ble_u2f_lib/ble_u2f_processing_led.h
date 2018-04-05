#ifndef BLE_U2F_PROCESSING_LED_H__
#define BLE_U2F_PROCESSING_LED_H__

#include <stdint.h>

#include "ble_u2f.h"

#ifdef __cplusplus
extern "C" {
#endif


void ble_u2f_processing_led_on(ble_u2f_t *p_u2f);
void ble_u2f_processing_led_off(ble_u2f_t *p_u2f);


#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_PROCESSING_LED_H__

/** @} */
