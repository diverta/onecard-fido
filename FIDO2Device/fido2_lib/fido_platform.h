/* 
 * File:   fido_platform.h
 * Author: makmorit
 *
 * Created on 2019/06/24, 10:45
 */
#ifndef FIDO_PLATFORM_H
#define FIDO_PLATFORM_H

#include <stdbool.h>
#include <stdint.h>

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

// キープアライブ・タイマー
#define U2F_KEEPALIVE_INTERVAL_MSEC   500
#define CTAP2_KEEPALIVE_INTERVAL_MSEC 200

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

//
// fido_command.c
//
void    fido_user_presence_terminate(void);
void    fido_user_presence_verify_start(uint32_t timeout_msec, void *p_context);
uint8_t fido_user_presence_verify_end(void);

//
// fido_log.h
//
// nRF52840の場合は、NRF_LOG_xxxx読替マクロが実体のため、
// nRF5 SDK依存のマクロ定義をインクルードさせる
#ifdef NRF52840_XXAA
#include "fido_log.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /* FIDO_PLATFORM_H */
