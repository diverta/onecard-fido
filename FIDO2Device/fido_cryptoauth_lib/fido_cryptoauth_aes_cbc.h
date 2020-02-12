/* 
 * File:   fido_cryptoauth_aes_cbc.h
 * Author: makmorit
 *
 * Created on 2020/02/03, 9:33
 */
#ifndef FIDO_CRYPTOAUTH_AES_CBC_H
#define FIDO_CRYPTOAUTH_AES_CBC_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool fido_cryptoauth_aes_cbc_new_password(void);
bool fido_cryptoauth_aes_cbc_encrypt(uint8_t *plaintext, uint8_t *encrypted, size_t *encrypt_size);
bool fido_cryptoauth_aes_cbc_decrypt(uint8_t *encrypted, uint8_t *decrypted, size_t *decrypt_size);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_CRYPTOAUTH_AES_CBC_H */
