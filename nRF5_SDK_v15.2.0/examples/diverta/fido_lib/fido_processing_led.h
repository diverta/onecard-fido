/* 
 * File:   fido_processing_led.h
 * Author: makmorit
 *
 * Created on 2018/12/18, 12:09
 */
#ifndef FIDO_PROCESSING_LED_H__
#define FIDO_PROCESSING_LED_H__

#ifdef __cplusplus
extern "C" {
#endif


void fido_processing_led_on(uint32_t led_for_processing);
void fido_processing_led_off(void);


#ifdef __cplusplus
}
#endif

#endif // FIDO_PROCESSING_LED_H__

/** @} */
