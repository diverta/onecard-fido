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
#include "fido_board_define.h"

#ifdef __cplusplus
extern "C" {
#endif

// 関数群
void fido_board_button_timers_init(void);
void fido_board_button_init(void);
void fido_board_led_pin_set(LED_COLOR led_color, bool led_on);
void fido_board_prepare_for_deep_sleep(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BOARD_H */
