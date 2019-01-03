/* 
 * File:   ctap_cbor_parse.h
 * Author: makmorit
 *
 * Created on 2018/12/25, 13:21
 */
#ifndef CTAP_CBOR_PARSE_H
#define CTAP_CBOR_PARSE_H

#include "cbor.h"

#ifdef __cplusplus
extern "C" {
#endif

// 各種処理用の定数
#define CLIENT_DATA_HASH_SIZE       32
#define RP_ID_MAX_SIZE              128
#define RP_NAME_MAX_SIZE            32
#define USER_ID_MAX_SIZE            64
#define USER_NAME_MAX_SIZE          65

// Public Key Credential Type
#define PUB_KEY_CRED_PUB_KEY        0x01
#define PUB_KEY_CRED_UNKNOWN        0x3F

// Values for COSE_Key format
//  Key type
#define COSE_KEY_LABEL_KTY          1
#define COSE_KEY_KTY_EC2            2
//  Curve type
#define COSE_KEY_LABEL_CRV          -1
#define COSE_KEY_CRV_P256           1
//  Key coordinate
#define COSE_KEY_LABEL_X            -2
#define COSE_KEY_LABEL_Y            -3
//  Signature algorithm
#define COSE_KEY_LABEL_ALG          3
#define COSE_ALG_ES256              -7

// Credential type suppurted or not
#define CREDENTIAL_IS_SUPPORTED     1
#define CREDENTIAL_NOT_SUPPORTED    0

typedef struct {
    uint8_t id[RP_ID_MAX_SIZE];
    uint8_t id_size;
    uint8_t name[RP_NAME_MAX_SIZE];
} CTAP_RP_ID_T;

typedef struct {
    uint8_t id[USER_ID_MAX_SIZE];
    uint8_t id_size;
    uint8_t name[USER_NAME_MAX_SIZE];
} CTAP_USER_ENTITY_T;

typedef struct {
    uint8_t publicKeyCredentialTypeName[16];
    uint8_t publicKeyCredentialType;
    int32_t COSEAlgorithmIdentifier;
} CTAP_PUBKEY_CRED_PARAM_T;

typedef struct {
    uint8_t rk;
    uint8_t uv;
    uint8_t up;
} CTAP_OPTIONS_T;

uint8_t parse_fixed_byte_string(CborValue *map, uint8_t *dst, int len);
uint8_t parse_rp_id(CTAP_RP_ID_T* rp, CborValue *val);
uint8_t parse_rp(CTAP_RP_ID_T *rp, CborValue *val);
uint8_t parse_user(CTAP_USER_ENTITY_T *user, CborValue *val);
uint8_t parse_pub_key_cred_params(CTAP_PUBKEY_CRED_PARAM_T *pubkey_cred_param, CborValue *val);
uint8_t parse_options(CTAP_OPTIONS_T *options, CborValue * val);

#ifdef __cplusplus
}
#endif

#endif /* CTAP_CBOR_PARSE_H */
