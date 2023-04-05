//
//  tool_ecdh.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/22.
//
#ifndef tool_ecdh_h
#define tool_ecdh_h

#include <stdio.h>
#include <string.h>
#include <stdint.h>

// 関数群
uint8_t  tool_ecdh_create_shared_secret_key(uint8_t *agreement_pubkey_X, uint8_t *agreement_pubkey_Y);
uint8_t *tool_ecdh_shared_secret_key(void);
uint8_t *tool_ecdh_public_key_X(void);
uint8_t *tool_ecdh_public_key_Y(void);

#endif /* tool_ecdh_h */
