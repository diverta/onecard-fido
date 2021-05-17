/* 
 * File:   app_crypto_ec.h
 * Author: makmorit
 *
 * Created on 2021/05/17, 12:06
 */
#ifndef APP_CRYPTO_EC_H
#define APP_CRYPTO_EC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool        app_crypto_ec_dsa_sign(uint8_t *private_key_be, uint8_t const *hash_digest, size_t digest_size, uint8_t *signature);
bool        app_crypto_ec_dsa_verify(uint8_t *public_key_be, uint8_t const *hash_digest, size_t digest_size, uint8_t *signature);

#ifdef __cplusplus
}
#endif

#endif /* APP_CRYPTO_EC_H */
