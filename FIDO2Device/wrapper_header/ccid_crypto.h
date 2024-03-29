/* 
 * File:   ccid_crypto.h
 * Author: makmorit
 *
 * Created on 2022/04/19, 10:12
 */
#ifndef CCID_CRYPTO_H
#define CCID_CRYPTO_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
uint8_t    *ccid_crypto_rsa_e_bytes(void);
uint8_t     ccid_crypto_rsa_e_size(void);
bool        ccid_crypto_rsa_private(uint8_t *rsa_private_key_raw, uint8_t *input, uint8_t *output);
bool        ccid_crypto_rsa_import(uint8_t *rsa_private_key_raw, uint8_t *rsa_public_key_raw, unsigned int nbits);
bool        ccid_crypto_rsa_generate_key(uint8_t *rsa_private_key_raw, uint8_t *rsa_public_key_raw, unsigned int nbits);

#ifdef __cplusplus
}
#endif

#endif /* CCID_CRYPTO_H */
