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

//
// ハッシュ計算関連
//
void     fido_command_calc_hash_sha256(uint8_t *data, size_t data_size, uint8_t *hash_digest, size_t *hash_digest_size);
void     fido_command_calc_hash_hmac_sha256(uint8_t *key_data, size_t key_data_size, uint8_t *src_data, size_t src_data_size, uint8_t *src_data_2, size_t src_data_2_size, uint8_t *dest_data);

//
// 公開鍵関連
//
bool     fido_command_keypair_generate(void);
uint8_t *fido_command_keypair_public_key(void);

//
// 共通鍵関連
//
void     fido_command_sskey_init(bool force);
uint8_t *fido_command_sskey_public_key(void);
uint8_t  fido_command_sskey_generate(uint8_t *client_public_key_raw_data);
size_t   fido_command_sskey_aes_256_cbc_decrypt(uint8_t *p_encrypted, size_t encrypted_size, uint8_t *decrypted);
size_t   fido_command_sskey_aes_256_cbc_encrypt(uint8_t *p_plaintext, size_t plaintext_size, uint8_t *encrypted);
void     fido_command_sskey_calculate_hmac_sha256(uint8_t *src_data, size_t src_data_size, uint8_t *src_data_2, size_t src_data_2_size, uint8_t *dest_data);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_COMMAND_COMMON_H */
