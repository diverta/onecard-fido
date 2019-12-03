/* 
 * File:   fido_cryptoauth.h
 * Author: makmorit
 *
 * Created on 2019/11/25, 14:43
 */
#ifndef FIDO_CRYPTOAUTH_H
#define FIDO_CRYPTOAUTH_H

#ifdef __cplusplus
extern "C" {
#endif

bool fido_cryptoauth_init(void);
void fido_cryptoauth_release(void);

void     fido_cryptoauth_keypair_generate(uint16_t key_id);
uint8_t *fido_cryptoauth_keypair_public_key(uint16_t key_id);
size_t   fido_cryptoauth_keypair_private_key_size(void);
void     fido_cryptoauth_generate_sha256_hash(uint8_t *data, size_t data_size, uint8_t *hash_digest, size_t *hash_digest_size);
void     fido_cryptoauth_generate_random_vector(uint8_t *vector_buf, size_t vector_buf_size);
void     fido_cryptoauth_ecdsa_sign(uint16_t key_id, uint8_t const *hash_digest, uint8_t *signature, size_t *signature_size);
bool     fido_cryptoauth_calculate_hmac_sha256(uint8_t *key_data, size_t key_data_size, uint8_t *src_data, size_t src_data_size, uint8_t *src_data_2, size_t src_data_2_size, uint8_t *dest_data);

void     fido_cryptoauth_sskey_init(bool force);
bool     fido_cryptoauth_sskey_generate(uint8_t *client_public_key_raw_data);
uint8_t *fido_cryptoauth_sskey_public_key(void);
uint8_t *fido_cryptoauth_sskey_hash(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_CRYPTOAUTH_H */
