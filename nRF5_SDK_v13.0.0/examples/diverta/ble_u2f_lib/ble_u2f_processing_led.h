#ifndef BLE_U2F_PROCESSING_LED_H__
#define BLE_U2F_PROCESSING_LED_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


void ble_u2f_processing_led_on(uint32_t led_for_processing);
void ble_u2f_processing_led_off(void);


#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_PROCESSING_LED_H__

/** @} */
