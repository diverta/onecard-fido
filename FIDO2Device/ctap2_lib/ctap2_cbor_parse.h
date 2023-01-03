/* 
 * File:   ctap_cbor_parse.h
 * Author: makmorit
 *
 * Created on 2018/12/25, 13:21
 */
#ifndef CTAP_CBOR_PARSE_H
#define CTAP_CBOR_PARSE_H

#include "ctap2_cbor.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t parse_fixed_byte_string(CborValue *map, uint8_t *dst, int len);
uint8_t parse_rp_id(void *p_ctap_rp_id, CborValue *val);
uint8_t parse_rp(void *p_ctap_rp_id, CborValue *val);
uint8_t parse_user(void *ctap_user_entity, CborValue *val);
uint8_t parse_pub_key_cred_params(void *ctap_pubkey_cred_param, CborValue *val);
uint8_t parse_options(void *ctap_options, CborValue *val, bool makeCredential);
uint8_t parse_verify_exclude_list(CborValue *val);
uint8_t parse_allow_list(void *ctap_allow_list, CborValue *it);
uint8_t parse_cose_pubkey(CborValue *it, void *ctap_cose_key);
uint8_t parse_extensions(CborValue *val, void *ctap_extensions);

#ifdef __cplusplus
}
#endif

#endif /* CTAP_CBOR_PARSE_H */
