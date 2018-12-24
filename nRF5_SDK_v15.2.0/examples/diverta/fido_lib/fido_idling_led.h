/* 
 * File:   fido_idling_led.h
 * Author: makmorit
 *
 * Created on 2018/12/18, 11:09
 */
#ifndef FIDO_IDLING_LED_H__
#define FIDO_IDLING_LED_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void fido_idling_led_on(uint32_t led_for_idling);
void fido_idling_led_off(uint32_t led_for_idling);

#ifdef __cplusplus
}
#endif

#endif // FIDO_IDLING_LED_H__

/** @} */
