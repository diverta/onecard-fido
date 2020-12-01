//
//  tool_crypto_private_key.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/01.
//
#ifndef tool_crypto_private_key_h
#define tool_crypto_private_key_h

#include <stdbool.h>
#include <stdio.h>

#define PIV_ALG_NONE                0x00
#define PIV_ALG_RSA2048             0x07
#define PIV_ALG_ECCP256             0x11

#define RSA2048_N_SIZE              2048
#define RSA2048_PQ_SIZE             128
#define RSA2048_PKEY_TLV_SIZE       655

#define ECCP256_KEY_SIZE            32
#define ECCP256_PKEY_TLV_SIZE       34

//
// public functions
//
bool             tool_crypto_private_key_extract_from_pem(const char *pem_path);
unsigned char   *tool_crypto_private_key_TLV_data(void);
size_t           tool_crypto_private_key_TLV_size(void);

#endif /* tool_crypto_private_key_h */
