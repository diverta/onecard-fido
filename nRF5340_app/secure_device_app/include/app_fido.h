/* 
 * File:   app_fido.h
 * Author: makmorit
 *
 * Created on 2021/09/16, 10:03
 */
#ifndef APP_FIDO_H
#define APP_FIDO_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        fido_crypto_generate_sha256_hash(uint8_t *data, size_t data_size, uint8_t *hash_digest, size_t *hash_digest_size);
void        fido_crypto_calculate_hmac_sha256(uint8_t *key_data, size_t key_data_size, uint8_t *src_data, size_t src_data_size, uint8_t *src_data_2, size_t src_data_2_size, uint8_t *dest_data);
uint8_t    *fido_crypto_sskey_public_key(void);
void        fido_crypto_sskey_init(bool force);
uint8_t     fido_crypto_sskey_generate(uint8_t *client_public_key_raw_data);
uint8_t    *fido_crypto_sskey_hash(void);
size_t      fido_crypto_aes_cbc_256_decrypt(uint8_t *p_key, uint8_t *p_encrypted, size_t encrypted_size, uint8_t *decrypted);
size_t      fido_crypto_aes_cbc_256_encrypt(uint8_t *p_key, uint8_t *p_plaintext, size_t plaintext_size, uint8_t *encrypted);

#ifdef __cplusplus
}
#endif

#endif /* APP_FIDO_H */
