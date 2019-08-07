/* 
 * File:   fido_timer.h
 * Author: makmorit
 *
 * Created on 2019/07/30, 11:09
 */
#ifndef FIDO_TIMER_H
#define FIDO_TIMER_H

#include <stdint.h>

void fido_timer_do_process(void);
void fido_comm_interval_timer_stop(void);
void fido_comm_interval_timer_start(void);
void fido_processing_led_timer_stop(void);
void fido_processing_led_timer_start(uint32_t on_off_interval_msec);
void fido_idling_led_timer_stop(void);
void fido_idling_led_timer_start(uint32_t on_off_interval_msec);
void fido_button_long_push_timer_init(void);
void fido_button_long_push_timer_stop(void);
void fido_button_long_push_timer_start(uint32_t timeout_msec, void *p_context);

//
// C --> CPP 呼出用インターフェース
//
void _fido_user_presence_verify_timer_stop(void);
void _fido_user_presence_verify_timer_start(uint32_t timeout_msec, void *p_context);
void _fido_keepalive_interval_timer_stop(void);
void _fido_keepalive_interval_timer_start(uint32_t timeout_msec, void *p_context);
void _fido_hid_channel_lock_timer_stop(void);
void _fido_hid_channel_lock_timer_start(uint32_t lock_ms);

#endif /* FIDO_TIMER_H */
