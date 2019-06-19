/* 
 * File:   nfc_fido_command.h
 * Author: makmorit
 *
 * Created on 2019/06/03, 15:20
 */
#ifndef NFC_FIDO_COMMAND_H
#define NFC_FIDO_COMMAND_H

#ifdef __cplusplus
extern "C" {
#endif

// for Flash ROM event
#include "fido_flash_event.h"

void nfc_fido_command_on_request_received(void);
void nfc_fido_command_on_fs_evt(fido_flash_event_t const *const p_evt);
void nfc_fido_command_on_send_completed(void);
void nfc_fido_command_on_request_started(void);

#ifdef __cplusplus
}
#endif

#endif /* NFC_FIDO_COMMAND_H */
