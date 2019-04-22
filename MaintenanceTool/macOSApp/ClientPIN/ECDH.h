//
//  ECDH.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/22.
//
#ifndef ECDH_h
#define ECDH_h

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "fido_blob.h"

/* COSE ES256 (ECDSA over P-256 with SHA-256) public key */
typedef struct es256_pk {
    unsigned char    x[32];
    unsigned char    y[32];
} es256_pk_t;

/* COSE ES256 (ECDSA over P-256 with SHA-256) (secret) key */
typedef struct es256_sk {
    unsigned char    d[32];
} es256_sk_t;

// 関数群
uint8_t  ECDH_create_shared_secret_key(uint8_t *agreement_pubkey_X, uint8_t *agreement_pubkey_Y);
uint8_t *ECDH_shared_secret_key(void);
uint8_t *ECDH_public_key_X(void);
uint8_t *ECDH_public_key_Y(void);

#endif /* ECDH_h */
