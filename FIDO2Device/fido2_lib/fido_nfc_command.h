/* 
 * File:   fido_nfc_command.h
 * Author: makmorit
 *
 * Created on 2019/06/03, 15:20
 */
#ifndef FIDO_NFC_COMMAND_H
#define FIDO_NFC_COMMAND_H

#ifdef __cplusplus
extern "C" {
#endif

// for Flash ROM event
#include "fido_flash_event.h"

void fido_nfc_command_on_fs_evt(fido_flash_event_t const *const p_evt);
void fido_nfc_command_on_send_completed(void);
void fido_nfc_command_on_request_started(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_NFC_COMMAND_H */
