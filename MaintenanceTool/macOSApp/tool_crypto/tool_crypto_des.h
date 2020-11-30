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
unsigned char   *tool_crypto_des_default_key(void);
bool             tool_crypto_des_import_key(const unsigned char *key_raw, const size_t key_raw_size);
bool             tool_crypto_des_decrypt(const unsigned char *input, const size_t input_size, unsigned char *output, size_t *output_size);

#endif /* tool_crypto_des_h */
