/* 
 * File:   ctap2_cbor.c
 * Author: makmorit
 *
 * Created on 2021/10/04, 9:30
 */
#ifdef FIDO_ZEPHYR
// version 0.5.0

#include "tinycbor/cbor.h"
#include "tinycbor/cbor_buf_reader.h"
#include "tinycbor/cbor_buf_writer.h"

static struct cbor_buf_reader reader;
static struct cbor_buf_writer writer;

void ctap2_cbor_encoder_init(CborEncoder *encoder, uint8_t *encoded_buff, size_t encoded_buff_size, int flags)
{
    cbor_buf_writer_init(&writer, encoded_buff, encoded_buff_size);
    cbor_encoder_init(encoder, &writer.enc, flags);
}

size_t ctap2_cbor_encoder_get_buffer_size(const CborEncoder *encoder, const uint8_t *encoded_buff)
{
    (void)encoder;
    return cbor_buf_writer_buffer_size(&writer, encoded_buff);
}

CborError ctap2_cbor_parser_init(const uint8_t *cbor_data_buffer, size_t cbor_data_size, uint32_t flags, CborParser *parser, CborValue *it)
{
    cbor_buf_reader_init(&reader, cbor_data_buffer, cbor_data_size);
    return cbor_parser_init(&reader.r, flags, parser, it);
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

CborError ctap2_cbor_parser_init(const uint8_t *cbor_data_buffer, size_t cbor_data_size, uint32_t flags, CborParser *parser, CborValue *it)
{
    return cbor_parser_init(cbor_data_buffer, cbor_data_size, flags, parser, it);
}

#endif
