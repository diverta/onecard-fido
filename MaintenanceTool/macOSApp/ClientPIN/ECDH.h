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

// 関数群
uint8_t  ECDH_create_shared_secret_key(uint8_t *agreement_pubkey_X, uint8_t *agreement_pubkey_Y);
uint8_t *ECDH_shared_secret_key(void);
uint8_t *ECDH_public_key_X(void);
uint8_t *ECDH_public_key_Y(void);

#endif /* ECDH_h */
