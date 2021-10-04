/* 
 * File:   ctap2_cbor.h
 * Author: makmorit
 *
 * Created on 2021/10/04, 9:30
 */
#ifndef CTAP2_CBOR_H
#define CTAP2_CBOR_H

#ifdef FIDO_ZEPHYR
// version 0.5.0
#include "tinycbor/cbor.h"

#else
// version 0.5.4
#include "cbor.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

void        ctap2_cbor_encoder_init(CborEncoder *encoder, uint8_t *encoded_buff, size_t encoded_buff_size, int flags);
size_t      ctap2_cbor_encoder_get_buffer_size(const CborEncoder *encoder, const uint8_t *encoded_buff);
CborError   ctap2_cbor_parser_init(const uint8_t *cbor_data_buffer, size_t cbor_data_size, uint32_t flags, CborParser *parser, CborValue *it);

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_CBOR_H */
