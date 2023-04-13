//
//  aes_256_cbc.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/28.
//
#ifndef aes_256_cbc_h
#define aes_256_cbc_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// 関数群
bool aes_256_cbc_enc(uint8_t *key, uint8_t *plain, size_t plain_size, uint8_t *encoded, size_t *encoded_size);
bool aes_256_cbc_dec(uint8_t *key, uint8_t *encoded, size_t encoded_size, uint8_t *decoded, size_t *decoded_size);

#endif /* aes_256_cbc_h */
