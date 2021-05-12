/* 
 * File:   app_crypto.h
 * Author: makmorit
 *
 * Created on 2021/05/12, 9:59
 */
#ifndef APP_CRYPTO_H
#define APP_CRYPTO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool app_crypto_aes_cbc_256_encrypt(uint8_t *p_key, uint8_t *p_plaintext, size_t plaintext_size, uint8_t *encrypted, size_t *encrypted_size);
bool app_crypto_aes_cbc_256_decrypt(uint8_t *p_key, uint8_t *p_encrypted, size_t encrypted_size, uint8_t *decrypted, size_t *decrypted_size);

#ifdef __cplusplus
}
#endif

#endif /* APP_CRYPTO_H */
