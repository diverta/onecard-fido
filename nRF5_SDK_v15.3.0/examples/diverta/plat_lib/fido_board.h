/* 
 * File:   fido_board.h
 * Author: makmorit
 *
 * Created on 2019/06/18, 10:12
 */

#ifndef FIDO_BOARD_H
#define FIDO_BOARD_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// LED点滅間隔の定義
#define LED_ON_OFF_INTERVAL_MSEC        300
#define LED_ON_OFF_SHORT_INTERVAL_MSEC  100
#define LED_BLINK_INTERVAL_MSEC         250

// 関数群
void fido_button_timers_init(void);
void fido_button_init(void);
void fido_processing_led_timedout_handler(void);
void fido_idling_led_timedout_handler(void);

void fido_led_light_all(bool led_on);
void fido_prompt_led_blink_start(uint32_t on_off_interval_msec);
void fido_caution_led_blink_start(uint32_t on_off_interval_msec);
void fido_led_blink_stop(void);
void fido_idling_led_blink_start(void);
void fido_idling_led_blink_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BOARD_H */

