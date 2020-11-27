//
//  tool_crypto_des.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/11/27.
//
#ifndef tool_crypto_des_h
#define tool_crypto_des_h

#include <stdbool.h>
#include <stdio.h>

#define DES_TYPE_3DES   1
#define DES_LEN_DES     8
#define DES_LEN_3DES    (DES_LEN_DES*3)

//
// public functions
//
unsigned char  *tool_crypto_des_default_key(void);
bool            tool_crypto_des_import_key(const unsigned char *keyraw, const size_t keyrawlen);

#endif /* tool_crypto_des_h */
