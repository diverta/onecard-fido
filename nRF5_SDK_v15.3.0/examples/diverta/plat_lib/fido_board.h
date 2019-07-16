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

// LED種別
typedef enum _LED_COLOR {
    LED_COLOR_NONE = 0,
    LED_COLOR_RED,
    LED_COLOR_GREEN,
    LED_COLOR_BLUE
} LED_COLOR;

// 関数群
void fido_button_timers_init(void);
void fido_button_init(void);
void fido_processing_led_timedout_handler(void);
void fido_idling_led_timedout_handler(void);
void led_light_pin_set(LED_COLOR led_color, bool led_on);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BOARD_H */
