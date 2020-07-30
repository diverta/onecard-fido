/* 
 * File:   fido_crypto.h
 * Author: makmorit
 *
 * Created on 2018/12/26, 12:19
 */
#ifndef FIDO_CRYPTO_H
#define FIDO_CRYPTO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void fido_crypto_init(void);
void fido_crypto_generate_sha256_hash(uint8_t *data, size_t data_size, uint8_t *hash_digest, size_t *hash_digest_size);
void fido_crypto_generate_random_vector(uint8_t *vector_buf, size_t vector_buf_size);
void fido_crypto_ecdsa_sign(uint8_t *private_key_be, uint8_t const *hash_digest, size_t digest_size, uint8_t *signature, size_t *signature_size);
bool fido_crypto_ecdsa_sign_verify(uint8_t *public_key_be, uint8_t const *hash_digest, size_t digest_size, uint8_t *signature, size_t signature_size);
void fido_crypto_calculate_hmac_sha256(uint8_t *key_data, size_t key_data_size, uint8_t *src_data, size_t src_data_size, uint8_t *src_data_2, size_t src_data_2_size, uint8_t *dest_data);
bool fido_crypto_calculate_ecdh(uint8_t *private_key_raw_data, uint8_t *client_public_key_raw_data, uint8_t *sskey_raw_data, size_t *sskey_raw_data_size);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_CRYPTO_H */

