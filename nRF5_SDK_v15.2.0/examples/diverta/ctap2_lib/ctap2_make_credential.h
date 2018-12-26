/* 
 * File:   ctap2_make_credential.h
 * Author: makmorit
 *
 * Created on 2018/12/25, 11:33
 */
#ifndef CTAP2_MAKE_CREDENTIAL_H
#define CTAP2_MAKE_CREDENTIAL_H

#ifdef __cplusplus
extern "C" {
#endif

uint8_t ctap2_make_credential_decode_request(uint8_t *cbor_data_buffer, size_t cbor_data_length);
uint8_t ctap2_make_credential_generate_response_items(void);

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_MAKE_CREDENTIAL_H */

