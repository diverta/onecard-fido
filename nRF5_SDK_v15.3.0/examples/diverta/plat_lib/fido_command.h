/* 
 * File:   fido_command.h
 * Author: makmorit
 *
 * Created on 2019/02/11, 11:31
 */
#ifndef FIDO_COMMAND_H
#define FIDO_COMMAND_H

#ifdef __cplusplus
extern "C" {
#endif

void fido_command_on_mainsw_event(void);
void fido_command_on_mainsw_long_push_event(void);
void fido_command_on_process_timedout(void);
void fido_command_long_push_timer_handler(void *p_context);
void fido_command_keepalive_timer_handler(void *p_context);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_COMMAND_H */

