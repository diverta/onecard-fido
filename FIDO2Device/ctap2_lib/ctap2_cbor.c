/* 
 * File:   ctap2_cbor.c
 * Author: makmorit
 *
 * Created on 2021/10/04, 9:30
 */
#ifdef FIDO_ZEPHYR
// version 0.5.0

#include "tinycbor/cbor.h"
#include "tinycbor/cbor_buf_writer.h"

static struct cbor_buf_writer writer;

void ctap2_cbor_encoder_init(CborEncoder *encoder, uint8_t *encoded_buff, size_t encoded_buff_size, int flags)
{
    cbor_buf_writer_init(&writer, encoded_buff, encoded_buff_size);
    cbor_encoder_init(encoder, &writer, flags);
}

size_t ctap2_cbor_encoder_get_buffer_size(const CborEncoder *encoder, const uint8_t *encoded_buff)
{
    cbor_buf_writer_buffer_size(encoder->writer, encoded_buff);
}

#else
// version 0.5.4

#include "cbor.h"

void ctap2_cbor_encoder_init(CborEncoder *encoder, uint8_t *encoded_buff, size_t encoded_buff_size, int flags)
{
    cbor_encoder_init(encoder, encoded_buff, encoded_buff_size, flags);
}

size_t ctap2_cbor_encoder_get_buffer_size(const CborEncoder *encoder, const uint8_t *encoded_buff)
{
    return cbor_encoder_get_buffer_size(encoder, encoded_buff);
}

#endif
