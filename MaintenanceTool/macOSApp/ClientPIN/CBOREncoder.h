//
//  CBOREncoder.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/18.
//
#ifndef CBOREncoder_h
#define CBOREncoder_h

#include <stdio.h>
#include "cbor.h"

uint8_t *ctap2_cbor_encode_request_bytes(void);
size_t   ctap2_cbor_encode_request_bytes_size(void);
uint8_t  ctap2_cbor_encode_get_agreement_key(void);
uint8_t  ctap2_cbor_encode_client_pin_set_or_change(
    uint8_t *agreement_pubkey_X, uint8_t *agreement_pubkey_Y, char *new_pin, char *old_pin);
uint8_t ctap2_cbor_encode_client_pin_token_get(
    uint8_t *agreement_pubkey_X, uint8_t *agreement_pubkey_Y, char *cur_pin);
uint8_t ctap2_cbor_encode_make_credential(
    uint8_t *agreement_pubkey_X, uint8_t *agreement_pubkey_Y, uint8_t *pin_token);
uint8_t ctap2_cbor_encode_get_assertion(
    uint8_t *agreement_pubkey_X, uint8_t *agreement_pubkey_Y, uint8_t *pin_token,
    uint8_t *credential_id, size_t credential_id_size, uint8_t *hmac_secret_salt);

#endif /* CBOREncoder_h */
