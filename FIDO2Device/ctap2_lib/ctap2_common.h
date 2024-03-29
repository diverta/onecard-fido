/* 
 * File:   ctap2_common.h
 * Author: makmorit
 *
 * Created on 2019/01/03, 11:32
 */
#ifndef CTAP2_COMMON_H
#define CTAP2_COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// 共通関数
uint8_t *ctap2_authenticator_data(size_t *size);
void     ctap2_authenticator_data_size_set(size_t size);
uint8_t *ctap2_generated_rpid_hash(void);
size_t   ctap2_generated_rpid_hash_size(void);
void     ctap2_generate_rpid_hash(uint8_t *rpid, size_t rpid_size);
void     ctap2_generate_signature_base(uint8_t *client_data_hash);
uint32_t ctap2_current_sign_count(void);
void     ctap2_set_sign_count(uint32_t count);
uint8_t  ctap2_flags_value(void);
void     ctap2_flags_init(uint8_t flag);
void     ctap2_flags_set(uint8_t flag);

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_COMMON_H */

