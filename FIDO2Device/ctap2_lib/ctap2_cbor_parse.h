/* 
 * File:   ctap_cbor_parse.h
 * Author: makmorit
 *
 * Created on 2018/12/25, 13:21
 */
#ifndef CTAP_CBOR_PARSE_H
#define CTAP_CBOR_PARSE_H

#include "ctap2_cbor.h"
#include "ctap2_common.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t parse_fixed_byte_string(CborValue *map, uint8_t *dst, int len);
uint8_t parse_rp_id(CTAP_RP_ID_T* rp, CborValue *val);
uint8_t parse_rp(CTAP_RP_ID_T *rp, CborValue *val);
uint8_t parse_user(CTAP_USER_ENTITY_T *user, CborValue *val);
uint8_t parse_pub_key_cred_params(CTAP_PUBKEY_CRED_PARAM_T *pubkey_cred_param, CborValue *val);
uint8_t parse_options(CTAP_OPTIONS_T *options, CborValue *val, bool makeCredential);
uint8_t parse_verify_exclude_list(CborValue *val);
uint8_t parse_allow_list(CTAP_ALLOW_LIST_T *allowList, CborValue *it);
uint8_t parse_cose_pubkey(CborValue *it, CTAP_COSE_KEY *cose_key);
uint8_t parse_extensions(CborValue *val, CTAP_EXTENSIONS_T *ext);

#ifdef __cplusplus
}
#endif

#endif /* CTAP_CBOR_PARSE_H */
