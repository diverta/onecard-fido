/* 
 * File:   fido_platform.h
 * Author: makmorit
 *
 * Created on 2019/06/24, 10:45
 */
#ifndef FIDO_PLATFORM_H
#define FIDO_PLATFORM_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ハードウェアの差異に依存しない定義を集約
#include "fido_platform_common.h"

//
// fido_crypto.c
//
void fido_crypto_generate_sha256_hash(uint8_t *data, size_t data_size, uint8_t *hash_digest, size_t *hash_digest_size);
void fido_crypto_generate_random_vector(uint8_t *vector_buf, size_t vector_buf_size);
void fido_crypto_ecdsa_sign(uint8_t *private_key_be, uint8_t const *hash_digest, size_t digest_size, uint8_t *signature, size_t *signature_size);
void fido_crypto_calculate_hmac_sha256(uint8_t *key_data, size_t key_data_size, uint8_t *src_data, size_t src_data_size, uint8_t *src_data_2, size_t src_data_2_size, uint8_t *dest_data);

//
// fido_crypto_keypair.c
//
void     fido_crypto_keypair_generate(void);
uint8_t *fido_crypto_keypair_private_key(void);
uint8_t *fido_crypto_keypair_public_key(void);
size_t   fido_crypto_keypair_private_key_size(void);

//
// fido_crypto_sskey.c
//
void     fido_crypto_sskey_init(bool force);
uint8_t  fido_crypto_sskey_generate(uint8_t *client_public_key_raw_data);
uint8_t *fido_crypto_sskey_public_key(void);
uint8_t *fido_crypto_sskey_hash(void);

//
// fido_flash_skey_cert.c
//
bool      fido_flash_skey_cert_delete(void);
bool      fido_flash_skey_cert_write(void);
bool      fido_flash_skey_cert_read(void);
bool      fido_flash_skey_cert_available(void);
bool      fido_flash_skey_cert_data_prepare(uint8_t *data, uint16_t length);
uint32_t *fido_flash_skey_cert_data(void);
uint8_t  *fido_flash_skey_data(void);
uint8_t  *fido_flash_cert_data(void);
uint32_t  fido_flash_cert_data_length(void);

//
// fido_flash_password.c
//
uint8_t *fido_flash_password_get(void);
bool     fido_flash_password_set(uint8_t *random_vector);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_PLATFORM_H */
