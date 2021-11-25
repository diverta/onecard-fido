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
#include "fido_common.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// fido_command.c
//
void fido_command_abort_flag_set(bool flag);

void fido_command_mainsw_event_handler(void);
void fido_user_presence_verify_timeout_handler(void);

void fido_user_presence_verify_start_on_reset(void);
void fido_user_presence_verify_start(uint32_t timeout_msec, void *context);
void fido_user_presence_verify_end(void);
void fido_user_presence_verify_cancel(void);
void fido_user_presence_verify_on_ble_scan_end(bool success);
void fido_user_presence_verify_end_message(const char *func_name, bool tup_done);

void fido_command_on_request_receive_completed(TRANSPORT_TYPE transport_type);
void fido_command_on_response_send_completed(TRANSPORT_TYPE transport_type);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_COMMAND_H */

