/* 
 * File:   fido_command.h
 * Author: makmorit
 *
 * Created on 2019/02/11, 11:31
 */
#ifndef FIDO_COMMAND_H
#define FIDO_COMMAND_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// fido_command.c
//
bool fido_command_do_abort(void);
void fido_command_abort_flag_set(bool flag);

void fido_command_mainsw_event_handler(void);
void fido_user_presence_verify_timeout_handler(void);
void fido_command_keepalive_timer_handler(void);

void fido_user_presence_verify_start(uint32_t timeout_msec);
void fido_user_presence_verify_end(void);
void fido_user_presence_verify_cancel(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_COMMAND_H */

