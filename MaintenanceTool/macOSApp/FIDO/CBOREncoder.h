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
uint8_t  ctap2_cbor_encode_generate_set_pin_cbor(bool change_pin, uint8_t *ecdh_public_key_x, uint8_t *ecdh_public_key_y,
    uint8_t *pin_auth, uint8_t *new_pin_enc, size_t new_pin_enc_size, uint8_t *pin_hash_enc);
uint8_t  ctap2_cbor_encode_generate_get_pin_token_cbor(uint8_t *ecdh_public_key_x, uint8_t *ecdh_public_key_y, uint8_t *pin_hash_enc);
uint8_t  ctap2_cbor_encode_generate_make_credential_cbor(void);
uint8_t  ctap2_cbor_encode_generate_get_assertion_cbor(
    uint8_t *credential_id, size_t credential_id_size, uint8_t *ecdh_public_key_x, uint8_t *ecdh_public_key_y, uint8_t *hmac_secret_salt, bool user_presence);

#endif /* CBOREncoder_h */
