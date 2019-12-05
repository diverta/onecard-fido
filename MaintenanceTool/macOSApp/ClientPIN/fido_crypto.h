//
//  fido_crypto.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/22.
//
#ifndef fido_crypto_h
#define fido_crypto_h

#include <stdio.h>
#include <stdbool.h>

// 関数群
uint8_t *pin_hash_enc(void);
uint8_t *new_pin_enc(void);
size_t   new_pin_enc_size(void);
uint8_t *pin_auth(void);
uint8_t *client_data_hash(void);
uint8_t *salt_enc(void);
uint8_t *salt_auth(void);

uint8_t  generate_pin_hash_enc(const char *cur_pin);
uint8_t  generate_new_pin_enc(const char *new_pin);
uint8_t  generate_pin_auth(bool change_pin);
uint8_t  generate_client_data_hash(const char *client_data);
uint8_t  generate_pin_auth_from_client_data(uint8_t *decrypted_pin_token, uint8_t *client_data_hash);
uint8_t  generate_salt_enc(uint8_t *hmac_secret_salt, size_t hmac_secret_salt_size);
uint8_t  generate_salt_auth(uint8_t *salt_enc, size_t salt_enc_size);

uint8_t  decrypto_pin_token(
            uint8_t *encrypted_pin_token, uint8_t *decrypted_pin_token, size_t pin_token_size);

uint8_t *skey_cert_bytes_enc(void);
size_t   skey_cert_bytes_enc_size(void);
uint8_t  generate_skey_cert_bytes_enc(uint8_t *skey_cert_bytes, size_t skey_cert_bytes_size);

#endif /* fido_crypto_h */
