/* 
 * File:   fido_command_common.h
 * Author: makmorit
 *
 * Created on 2020/02/06, 12:21
 */
#ifndef FIDO_COMMAND_COMMON_H
#define FIDO_COMMAND_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool     fido_command_check_skey_cert_exist(void);
bool     fido_command_check_aes_password_exist(void);
size_t   fido_command_aes_cbc_decrypt(uint8_t *p_encrypted, size_t encrypted_size, uint8_t *decrypted);
size_t   fido_command_aes_cbc_encrypt(uint8_t *p_plaintext, size_t plaintext_size, uint8_t *encrypted);
void     fido_command_sskey_init(bool force);
uint8_t *fido_command_keypair_public_key(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_COMMAND_COMMON_H */
