//
//  CBOREncoder.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/18.
//
#ifndef CBOREncoder_h
#define CBOREncoder_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

uint8_t *ctap2_cbor_encode_request_bytes(void);
size_t   ctap2_cbor_encode_request_bytes_size(void);
uint8_t  ctap2_cbor_encode_get_agreement_key(void);
uint8_t  ctap2_cbor_encode_client_pin_set_or_change(char *new_pin, char *old_pin, uint8_t *ecdh_public_key_x, uint8_t *ecdh_public_key_y);
uint8_t  ctap2_cbor_encode_client_pin_token_get(char *cur_pin, uint8_t *ecdh_public_key_x, uint8_t *ecdh_public_key_y);
uint8_t  ctap2_cbor_encode_make_credential(uint8_t *pin_token);
uint8_t  ctap2_cbor_encode_get_assertion(
    uint8_t *pin_token, uint8_t *credential_id, size_t credential_id_size,
    uint8_t *ecdh_public_key_x, uint8_t *ecdh_public_key_y, uint8_t *hmac_secret_salt, bool user_presence);

#endif /* CBOREncoder_h */
