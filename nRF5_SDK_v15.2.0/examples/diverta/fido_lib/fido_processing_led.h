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

// 点滅間隔の定義
#define LED_ON_OFF_INTERVAL_MSEC 300
#define LED_ON_OFF_SHORT_INTERVAL_MSEC 100

void fido_processing_led_on(uint32_t led_for_processing, uint32_t on_off_interval_msec);
void fido_processing_led_off(void);


#ifdef __cplusplus
}
#endif

#endif // FIDO_PROCESSING_LED_H__

/** @} */
