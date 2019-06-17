/* 
 * File:   fido_crypto.h
 * Author: makmorit
 *
 * Created on 2018/12/26, 12:19
 */
#ifndef FIDO_CRYPTO_H
#define FIDO_CRYPTO_H

#include "nrf_crypto_hash.h"
#include "nrf_crypto_ecdsa.h"

#ifdef __cplusplus
extern "C" {
#endif

// サイズの定義（nRF5 SDKの定義から抽象化）
#define ECDSA_SIGNATURE_SIZE    NRF_CRYPTO_ECDSA_SIGNATURE_MAX_SIZE
#define SHA_256_HASH_SIZE       NRF_CRYPTO_HASH_SIZE_SHA256
#define SSKEY_HASH_SIZE         NRF_CRYPTO_HASH_SIZE_SHA256    
#define HMAC_SHA_256_SIZE       NRF_CRYPTO_HASH_SIZE_SHA256    

void fido_crypto_init(void);
void fido_crypto_generate_sha256_hash(uint8_t *data, size_t data_size, nrf_crypto_hash_sha256_digest_t hash_digest, size_t *hash_digest_size);
void fido_crypto_generate_random_vector(uint8_t *vector_buf, size_t vector_buf_size);
void fido_crypto_ecdsa_sign(uint8_t *private_key_be, uint8_t const *hash_digest, size_t digest_size, uint8_t *signature, size_t *signature_size);
void fido_crypto_calculate_hmac_sha256(uint8_t *key_data, size_t key_data_size, uint8_t *src_data, size_t src_data_size, uint8_t *src_data_2, size_t src_data_2_size, uint8_t *dest_data);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_CRYPTO_H */

