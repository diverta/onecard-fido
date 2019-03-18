/* 
 * File:   ctap2_get_assertion.h
 * Author: makmorit
 *
 * Created on 2019/01/03, 11:05
 */
#ifndef CTAP2_GET_ASSERTION_H
#define CTAP2_GET_ASSERTION_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t ctap2_get_assertion_decode_request(uint8_t *cbor_data_buffer, size_t cbor_data_length);
bool    ctap2_get_assertion_is_tup_needed(void);
uint8_t ctap2_get_assertion_generate_response_items(void);
uint8_t ctap2_get_assertion_encode_response(uint8_t *encoded_buff, size_t *encoded_buff_size);
uint8_t ctap2_get_assertion_update_token_counter(void);
uint8_t ctap2_get_assertion_verify_pin_auth(void);

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_GET_ASSERTION_H */

