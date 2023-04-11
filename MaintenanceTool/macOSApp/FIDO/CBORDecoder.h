//
//  CBORDecoder.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/22.
//
#ifndef CBORDecoder_h
#define CBORDecoder_h

#include <stdio.h>
#include "cbor.h"

// Values for COSE_Key format
//  Key type
#define COSE_KEY_LABEL_KTY          1
#define COSE_KEY_KTY_EC2            2
//  Signature algorithm
#define COSE_KEY_LABEL_ALG          3
#define COSE_ALG_ES256              -7
//  Curve type
#define COSE_KEY_LABEL_CRV          -1
#define COSE_KEY_CRV_P256           1
//  Key coordinate
#define COSE_KEY_LABEL_X            -2
#define COSE_KEY_LABEL_Y            -3

// データ長
#define RP_ID_HASH_SIZE             32
#define SIGN_COUNT_SIZE             4
#define AAGUID_SIZE                 16
#define CREDENTIAL_ID_LENGTH_SIZE   2
#define CREDENTIAL_ID_MAX_SIZE      128
#define CREDENTIAL_PUBKEY_MAX_SIZE  77
#define EXT_CBOR_FOR_CRED_MAX_SIZE  14
#define EXT_CBOR_FOR_GET_MAX_SIZE   79

typedef struct {
    int kty;
    int alg;
    int crv;
    struct {
        uint8_t x[32];
        uint8_t y[32];
    } key;
} CTAP_COSE_KEY;

typedef struct {
    uint8_t  rpIdHash[RP_ID_HASH_SIZE];
    uint8_t  flags;
    uint32_t signCount;
    uint8_t  aaguid[AAGUID_SIZE];
    size_t   credentialIdLength;
    uint8_t  credentialId[CREDENTIAL_ID_MAX_SIZE];
    uint8_t  credentialPubKey[CREDENTIAL_PUBKEY_MAX_SIZE];
} CTAP_MAKE_CREDENTIAL_RES;

typedef struct {
    bool     flag;
    uint8_t  output[64];
    size_t   output_size;
} CTAP_EXT_HMAC_SECRET_RES;

uint8_t  ctap2_cbor_decode_get_agreement_key(uint8_t *cbor_data_buffer, size_t cbor_data_length);
uint8_t *ctap2_cbor_decode_agreement_pubkey_X(void);
uint8_t *ctap2_cbor_decode_agreement_pubkey_Y(void);

uint8_t  ctap2_cbor_decode_pin_token(uint8_t *cbor_data_buffer, size_t cbor_data_length);
uint8_t *ctap2_cbor_decrypted_pin_token(void);

uint8_t  ctap2_cbor_decode_make_credential(uint8_t *cbor_data_buffer, size_t cbor_data_length);
uint8_t  ctap2_cbor_decode_get_assertion(uint8_t *cbor_data_buffer, size_t cbor_data_length, bool verify_salt);
uint8_t *ctap2_cbor_decode_credential_id(void);
size_t   ctap2_cbor_decode_credential_id_size(void);
bool     ctap2_cbor_decode_verify_salt(void);
CTAP_EXT_HMAC_SECRET_RES *ctap2_cbor_decode_ext_hmac_secret(void);

#endif /* CBORDecoder_h */
