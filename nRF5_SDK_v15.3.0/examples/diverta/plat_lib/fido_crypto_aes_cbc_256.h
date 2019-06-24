/* 
 * File:   fido_crypto_aes_cbc_256.h
 * Author: makmorit
 *
 * Created on 2019/02/25, 15:11
 */
#ifndef FIDO_CRYPTO_AES_CBC_256_H
#define FIDO_CRYPTO_AES_CBC_256_H

#ifdef __cplusplus
extern "C" {
#endif

size_t fido_crypto_aes_cbc_256_decrypt(uint8_t *p_key, uint8_t *p_encrypted, size_t encrypted_size, uint8_t *decrypted);
size_t fido_crypto_aes_cbc_256_encrypt(uint8_t *p_key, uint8_t *p_plaintext, size_t plaintext_size, uint8_t *encrypted);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_CRYPTO_AES_CBC_256_H */
