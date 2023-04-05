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

//
// public functions
//
bool             tool_crypto_private_key_extract_from_pem(const char *pem_path, unsigned char *alg, unsigned char *pkey_data, size_t *pkey_size);

#endif /* tool_crypto_private_key_h */
