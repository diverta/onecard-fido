/* 
 * File:   ctap2_cbor_encode.h
 * Author: makmorit
 *
 * Created on 2019/02/18, 15:31
 */
#ifndef CTAP2_CBOR_ENCODE_H
#define CTAP2_CBOR_ENCODE_H

#ifdef __cplusplus
extern "C" {
#endif

uint8_t ctap2_cbor_encode_response_retry_counter(uint8_t *encoded_buff, size_t *encoded_buff_size, uint32_t retry_counter);
uint8_t ctap2_cbor_encode_response_key_agreement(uint8_t *encoded_buff, size_t *encoded_buff_size);
uint8_t ctap2_cbor_encode_response_set_pin(uint8_t *encoded_buff, size_t *encoded_buff_size);

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_CBOR_ENCODE_H */
