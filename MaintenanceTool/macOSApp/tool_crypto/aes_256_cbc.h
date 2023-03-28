//
//  aes_256_cbc.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/28.
//
#ifndef aes_256_cbc_h
#define aes_256_cbc_h

#include <stddef.h>
#include <stdint.h>

// 関数群
uint8_t aes_256_cbc_enc(uint8_t *key, uint8_t *decoded, uint8_t *encoded, size_t *size);
uint8_t aes_256_cbc_dec(uint8_t *key, uint8_t *encoded, uint8_t *decoded, size_t *size);

#endif /* aes_256_cbc_h */
