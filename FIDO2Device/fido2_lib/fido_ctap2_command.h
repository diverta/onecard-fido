/* 
 * File:   fido_ctap2_command.h
 * Author: makmorit
 *
 * Created on 2018/12/18, 13:36
 */
#ifndef FIDO_CTAP2_COMMAND_H
#define FIDO_CTAP2_COMMAND_H

#include "fido_common.h"

// for Flash ROM event
#include "fido_flash_event.h"

#ifdef __cplusplus
extern "C" {
#endif

void fido_ctap2_command_hid_init(void);
void fido_ctap2_command_send_response(uint8_t ctap2_status, size_t length);
void fido_ctap2_command_cbor(TRANSPORT_TYPE transport_type);
void fido_ctap2_command_cbor_send_response(fido_flash_event_t const *const p_evt);
void fido_ctap2_command_cbor_response_completed(void);
void fido_ctap2_command_tup_cancel(void);
void fido_ctap2_command_cancel(void);
void fido_ctap2_command_keepalive_timer_handler(void);
bool fido_ctap2_command_on_mainsw_event(void);
bool fido_ctap2_command_on_mainsw_long_push_event(void);
void fido_ctap2_command_on_ble_nus_connected(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_CTAP2_COMMAND_H */

