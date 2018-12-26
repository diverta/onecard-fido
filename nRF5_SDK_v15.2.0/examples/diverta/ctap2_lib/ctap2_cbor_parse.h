/* 
 * File:   ctap_cbor_parse.h
 * Author: makmorit
 *
 * Created on 2018/12/25, 13:21
 */
#ifndef CTAP_CBOR_PARSE_H
#define CTAP_CBOR_PARSE_H

#include "cbor.h"
#include "ctap2.h"

#ifdef __cplusplus
extern "C" {
#endif

// 各種処理用の定数
#define CLIENT_DATA_HASH_SIZE       32
#define DOMAIN_NAME_MAX_SIZE        253
#define RP_NAME_LIMIT               32
#define USER_ID_MAX_SIZE            64
#define USER_NAME_LIMIT             65

// Public Key Credential Type
#define PUB_KEY_CRED_PUB_KEY        0x01
#define PUB_KEY_CRED_UNKNOWN        0x3F

// COSE Algorithm Identifier
#define COSE_ALG_ES256              -7

// Credential type suppurted or not
#define CREDENTIAL_IS_SUPPORTED     1
#define CREDENTIAL_NOT_SUPPORTED    0

typedef struct {
    uint8_t id[DOMAIN_NAME_MAX_SIZE];
    uint8_t id_size;
    uint8_t name[RP_NAME_LIMIT];
} CTAP_RP_ID_T;

typedef struct {
    uint8_t id[USER_ID_MAX_SIZE];
    uint8_t id_size;
    uint8_t name[USER_NAME_LIMIT];
} CTAP_USER_ENTITY_T;

typedef struct {
    uint8_t publicKeyCredentialTypeName[16];
    uint8_t publicKeyCredentialType;
    int32_t COSEAlgorithmIdentifier;
} CTAP_PUBKEY_CRED_PARAM_T;

uint8_t parse_fixed_byte_string(CborValue *map, uint8_t *dst, int len);
uint8_t parse_rp_id(CTAP_RP_ID_T* rp, CborValue *val);
uint8_t parse_rp(CTAP_RP_ID_T *rp, CborValue *val);
uint8_t parse_user(CTAP_USER_ENTITY_T *user, CborValue *val);
uint8_t parse_pub_key_cred_params(CTAP_PUBKEY_CRED_PARAM_T *pubkey_cred_param, CborValue *val);

#ifdef __cplusplus
}
#endif

#endif /* CTAP_CBOR_PARSE_H */
