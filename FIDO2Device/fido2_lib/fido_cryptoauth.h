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

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

//
// 使用するスロット番号の定義    
//
//  8: HMAC-SHA-256ハッシュ生成用のキーを一時収容
//  9: ECDH共通鍵を生成するためのセッション用秘密鍵を収容
// 14: FIDO認証器固有の秘密鍵を収容
// 15: 14番スロットを暗号化するためのキーを収容
#define KEY_ID_FOR_HMAC_GENERATE_KEY    8
#define KEY_ID_FOR_SHARED_SECRET_KEY    9
#define KEY_ID_FOR_INSTALL_PRIVATE_KEY  14
#define KEY_ID_FOR_INSTALL_PRV_TMP_KEY  15

bool     fido_cryptoauth_init(void);
void     fido_cryptoauth_release(void);
bool     fido_cryptoauth_get_config_bytes(void);

void     fido_cryptoauth_keypair_generate(uint16_t key_id);
uint8_t *fido_cryptoauth_keypair_public_key(uint16_t key_id);
void     fido_cryptoauth_generate_sha256_hash(uint8_t *data, size_t data_size, uint8_t *hash_digest, size_t *hash_digest_size);
void     fido_cryptoauth_generate_random_vector(uint8_t *vector_buf, size_t vector_buf_size);
void     fido_cryptoauth_ecdsa_sign(uint16_t key_id, uint8_t const *hash_digest, uint8_t *signature, size_t *signature_size);
bool     fido_cryptoauth_calculate_hmac_sha256(uint8_t *key_data, size_t key_data_size, uint8_t *src_data, size_t src_data_size, uint8_t *src_data_2, size_t src_data_2_size, uint8_t *dest_data);

void     fido_cryptoauth_sskey_init(bool force);
bool     fido_cryptoauth_sskey_generate(uint8_t *client_public_key_raw_data);
uint8_t *fido_cryptoauth_sskey_public_key(void);
uint8_t *fido_cryptoauth_sskey_hash(void);

bool     fido_cryptoauth_install_privkey(uint8_t *private_key_raw_data);
bool     fido_cryptoauth_extract_pubkey_from_cert(uint8_t *public_key, uint8_t *cert_data, size_t cert_data_length);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_CRYPTOAUTH_H */
