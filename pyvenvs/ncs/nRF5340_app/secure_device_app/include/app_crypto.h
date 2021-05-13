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
bool        app_crypto_aes_cbc_256_encrypt(uint8_t *p_key, uint8_t *p_plaintext, size_t plaintext_size, uint8_t *encrypted, size_t *encrypted_size);
bool        app_crypto_aes_cbc_256_decrypt(uint8_t *p_key, uint8_t *p_encrypted, size_t encrypted_size, uint8_t *decrypted, size_t *decrypted_size);
bool        app_crypto_generate_random_vector(uint8_t *vector_buf, size_t vector_size);
bool        app_crypto_generate_sha256_hash(uint8_t *data, size_t data_size, uint8_t *hash_digest);
bool        app_crypto_generate_hmac_sha256(uint8_t *key_data, size_t key_data_size, uint8_t *src_data, size_t src_data_size, uint8_t *src_data_2, size_t src_data_2_size, uint8_t *hash_digest);
bool        app_crypto_des3_ecb(const uint8_t *in, uint8_t *out, const uint8_t *key);

#ifdef __cplusplus
}
#endif

#endif /* APP_CRYPTO_H */
