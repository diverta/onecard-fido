/* 
 * File:   fido_nfc_send.h
 * Author: makmorit
 *
 * Created on 2019/05/29, 11:30
 */
#ifndef FIDO_NFC_SEND_H
#define FIDO_NFC_SEND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "fido_nfc_common.h"

// Capability Container
typedef struct {
    uint8_t cclen_hi;
    uint8_t cclen_lo;
    uint8_t version;
    uint8_t MLe_hi;
    uint8_t MLe_lo;
    uint8_t MLc_hi;
    uint8_t MLc_lo;
    uint8_t tlv[8];
} __attribute__((packed)) CAPABILITY_CONTAINER;

//
// 関数群
//
bool nfc_fido_send_response_ex(uint8_t *data, uint8_t data_size, uint16_t status_word);
bool nfc_fido_send_response(uint16_t resp);
void nfc_fido_send_app_selection_response(NFC_APPLETS selected_app);
void nfc_fido_send_ndef_cc_sample(void);
void nfc_fido_send_ndef_tag_sample(APDU_HEADER *apdu);
void nfc_fido_send_command_response_cont(uint8_t get_response_size);
void nfc_fido_send_command_response(uint8_t *response_buffer, size_t response_length);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_NFC_SEND_H */
