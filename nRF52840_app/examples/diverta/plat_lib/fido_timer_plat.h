/* 
 * File:   fido_timer_plat.h
 * Author: makmorit
 *
 * Created on 2019/06/18, 11:00
 */
#ifndef FIDO_TIMER_PLAT_H
#define FIDO_TIMER_PLAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void fido_comm_interval_timer_stop(void);
void fido_comm_interval_timer_start(uint32_t timeout_msec, void (*_handler)(void));
void fido_processing_led_timer_stop(void);
void fido_processing_led_timer_start(uint32_t on_off_interval_msec, void (*_handler)(void));
void fido_idling_led_timer_stop(void);
void fido_idling_led_timer_start(uint32_t on_off_interval_msec, void (*_handler)(void));
void fido_button_long_push_timer_init(void);
void fido_button_long_push_timer_stop(void);
void fido_button_long_push_timer_start(uint32_t timeout_msec, void (*_handler)(void));

#ifdef __cplusplus
}
#endif

#endif /* FIDO_TIMER_PLAT_H */
