/* 
 * File:   fido_crypto.h
 * Author: makmorit
 *
 * Created on 2021/09/16, 10:03
 */
#ifndef FIDO_CRYPTO_H
#define FIDO_CRYPTO_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        fido_crypto_generate_sha256_hash(uint8_t *data, size_t data_size, uint8_t *hash_digest, size_t *hash_digest_size);
void        fido_crypto_calculate_hmac_sha256(uint8_t *key_data, size_t key_data_size, uint8_t *src_data, size_t src_data_size, uint8_t *src_data_2, size_t src_data_2_size, uint8_t *dest_data);
void        fido_crypto_generate_random_vector(uint8_t *vector_buf, size_t vector_buf_size);
void        fido_crypto_keypair_generate(void);
uint8_t    *fido_crypto_keypair_private_key(void);
uint8_t    *fido_crypto_keypair_public_key(void);
bool        fido_crypto_ecdsa_sign(uint8_t *private_key_be, uint8_t const *hash_digest, size_t digest_size, uint8_t *signature, size_t *signature_size);
bool        fido_crypto_ecdsa_sign_verify(uint8_t *public_key_be, uint8_t const *hash_digest, size_t digest_size, uint8_t *signature, size_t signature_size);
uint8_t    *fido_crypto_sskey_public_key(void);
void        fido_crypto_sskey_init(bool force);
uint8_t     fido_crypto_sskey_generate(uint8_t *client_public_key_raw_data);
uint8_t    *fido_crypto_sskey_hash(void);
size_t      fido_crypto_aes_cbc_256_decrypt(uint8_t *p_key, uint8_t *p_encrypted, size_t encrypted_size, uint8_t *decrypted);
size_t      fido_crypto_aes_cbc_256_encrypt(uint8_t *p_key, uint8_t *p_plaintext, size_t plaintext_size, uint8_t *encrypted);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_CRYPTO_H */
