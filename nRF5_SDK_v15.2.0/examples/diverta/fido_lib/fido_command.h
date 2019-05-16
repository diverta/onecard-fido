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

void fido_button_timers_init(void);
void fido_button_init(void);
void fido_command_fds_register(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_COMMAND_H */

