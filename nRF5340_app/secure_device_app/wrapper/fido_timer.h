/* 
 * File:   fido_timer.h
 * Author: makmorit
 *
 * Created on 2021/09/21, 11:43
 */
#ifndef FIDO_TIMER_H
#define FIDO_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        fido_repeat_process_timer_stop(void);
void        fido_repeat_process_timer_start(uint32_t timeout_msec, void (*handler)(void));
void        fido_user_presence_verify_timer_stop(void);
void        fido_user_presence_verify_timer_start(uint32_t timeout_msec, void *p_context);
void        fido_hid_channel_lock_timer_stop(void);
void        fido_hid_channel_lock_timer_start(uint32_t lock_ms);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_TIMER_H */
