/* 
 * File:   fido_ctap2_command.h
 * Author: makmorit
 *
 * Created on 2018/12/18, 13:36
 */
#ifndef FIDO_CTAP2_COMMAND_H
#define FIDO_CTAP2_COMMAND_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "fido_common.h"

#ifdef __cplusplus
extern "C" {
#endif

void fido_ctap2_command_hid_init(void);
void fido_ctap2_command_send_response(uint8_t ctap2_status, size_t length);
void fido_ctap2_command_cbor(TRANSPORT_TYPE transport_type);
void fido_ctap2_command_tup_cancel(void);
void fido_ctap2_command_cancel(void);
void fido_ctap2_command_keepalive_timer_handler(void);
bool fido_ctap2_command_on_mainsw_event(void);
bool fido_ctap2_command_on_mainsw_long_push_event(void);
void fido_ctap2_command_on_ble_nus_connected(void);
void fido_ctap2_command_flash_failed(void);
void fido_ctap2_command_flash_gc_done(void);
void fido_ctap2_command_token_counter_file_deleted(void);
void fido_ctap2_command_retry_counter_record_updated(void);
void fido_ctap2_command_token_counter_record_updated(void);
void fido_ctap2_command_cbor_response_sent(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_CTAP2_COMMAND_H */

