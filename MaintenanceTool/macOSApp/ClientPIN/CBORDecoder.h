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

typedef struct {
    int kty;
    int alg;
    int crv;
    struct {
        uint8_t x[32];
        uint8_t y[32];
    } key;
} CTAP_COSE_KEY;

uint8_t  ctap2_cbor_decode_get_agreement_key(uint8_t *cbor_data_buffer, size_t cbor_data_length);
uint8_t *ctap2_cbor_decode_agreement_pubkey_X(void);
uint8_t *ctap2_cbor_decode_agreement_pubkey_Y(void);

#endif /* CBORDecoder_h */
