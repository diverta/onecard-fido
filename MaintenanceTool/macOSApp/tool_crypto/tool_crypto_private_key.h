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
// utility function
//
size_t           tool_crypto_tlv_set_length(unsigned char *buffer, size_t length);

//
// public functions
//
bool             tool_crypto_private_key_extract_from_pem(const char *pem_path);
unsigned char   *tool_crypto_private_key_APDU_data(void);
size_t           tool_crypto_private_key_APDU_size(void);

#endif /* tool_crypto_private_key_h */
