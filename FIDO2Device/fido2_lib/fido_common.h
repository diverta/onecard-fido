/* 
 * File:   fido_common.h
 * Author: makmorit
 *
 * Created on 2018/12/18, 11:09
 */
#ifndef FIDO_COMMON_H
#define FIDO_COMMON_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// 関数群
void     fido_set_status_word(uint8_t *dest_buffer, uint16_t status_word);
void     fido_set_uint32_bytes(uint8_t *p_dest_buffer, uint32_t bytes);
void     fido_set_uint16_bytes(uint8_t *p_dest_buffer, uint16_t bytes);
uint16_t fido_get_uint16_from_bytes(uint8_t *p_src_buffer);
uint32_t fido_get_uint32_from_bytes(uint8_t *p_src_buffer);
uint64_t fido_get_uint64_from_bytes(uint8_t *p_src_buffer);
size_t   fido_calculate_aes_block_size(size_t buffer_size);
uint8_t *fido_extract_pubkey_in_certificate(uint8_t *cert_data, size_t cert_data_length);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_COMMON_H */
