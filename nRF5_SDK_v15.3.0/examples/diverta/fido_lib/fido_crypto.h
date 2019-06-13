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

void fido_crypto_init(void);
void fido_crypto_generate_sha256_hash(uint8_t *data, size_t data_size, nrf_crypto_hash_sha256_digest_t hash_digest, size_t *hash_digest_size);
void fido_crypto_generate_random_vector(uint8_t *vector_buf, size_t vector_buf_size);
void fido_crypto_ecdsa_sign(nrf_crypto_ecc_private_key_t *private_key_for_sign, uint8_t const *hash_digest, size_t digest_size, uint8_t *signature, size_t *signature_size);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_CRYPTO_H */

