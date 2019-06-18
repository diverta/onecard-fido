/* 
 * File:   fido_command.h
 * Author: makmorit
 *
 * Created on 2019/02/11, 11:31
 */
#ifndef FIDO_COMMAND_H
#define FIDO_COMMAND_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// キープアライブ・タイマー
#define U2F_KEEPALIVE_INTERVAL_MSEC   500
#define CTAP2_KEEPALIVE_INTERVAL_MSEC 200

void fido_command_on_mainsw_event(void);
void fido_command_on_mainsw_long_push_event(void);
void fido_command_fds_register(void);
void fido_command_on_process_timedout(void);
void fido_command_long_push_timer_handler(void *p_context);
void fido_command_keepalive_timer_handler(void *p_context);

void    fido_user_presence_terminate(void);
void    fido_user_presence_verify_start(uint32_t timeout_msec, void *p_context);
uint8_t fido_user_presence_verify_end(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_COMMAND_H */

