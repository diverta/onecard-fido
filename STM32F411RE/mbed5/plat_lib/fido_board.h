/* 
 * File:   fido_board.h
 * Author: makmorit
 *
 * Created on 2019/07/30, 16:18
 */
#ifndef FIDO_BOARD_H
#define FIDO_BOARD_H

#include <stdbool.h>
#include <stdint.h>

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
void fido_command_long_push_timer_handler(void *p_context);
void led_light_pin_set(LED_COLOR led_color, bool led_on);

#endif /* FIDO_BOARD_H */
