/* 
 * File:   ctap2_make_credential.h
 * Author: makmorit
 *
 * Created on 2018/12/25, 11:33
 */
#ifndef CTAP2_MAKE_CREDENTIAL_H
#define CTAP2_MAKE_CREDENTIAL_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t ctap2_make_credential_decode_request(uint8_t *cbor_data_buffer, size_t cbor_data_length);
bool    ctap2_make_credential_is_tup_needed(void);
uint8_t ctap2_make_credential_generate_response_items(void);
uint8_t ctap2_make_credential_encode_response(uint8_t *encoded_buff, size_t *encoded_buff_size);
uint8_t ctap2_make_credential_add_token_counter(void);
uint8_t ctap2_make_credential_verify_pin_auth(void);

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_MAKE_CREDENTIAL_H */

