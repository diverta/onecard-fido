/* 
 * File:   fido_platform.h
 * Author: makmorit
 *
 * Created on 2019/06/24, 10:45
 */
#ifndef FIDO_PLATFORM_H
#define FIDO_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

// LED点灯モード
typedef enum _LED_LIGHT_MODE {
    LED_LIGHT_NONE = 0,
    LED_LIGHT_FOR_PAIRING_MODE,
    LED_LIGHT_FOR_USER_PRESENCE,
    LED_LIGHT_FOR_PROCESSING
} LED_LIGHT_MODE;

// LED点滅間隔の定義
#define LED_ON_OFF_INTERVAL_MSEC        300
#define LED_ON_OFF_SHORT_INTERVAL_MSEC  100
#define LED_BLINK_INTERVAL_MSEC         250

//
// fido_board.c
//
void fido_button_timers_init(void);
void fido_button_init(void);
void fido_led_light(LED_LIGHT_MODE led_light_mode, bool led_on);
void fido_led_light_all(bool led_on);
void fido_processing_led_on(LED_LIGHT_MODE led_light_mode, uint32_t on_off_interval_msec);
void fido_processing_led_off(void);
void fido_idling_led_on(void);
void fido_idling_led_off(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_PLATFORM_H */
