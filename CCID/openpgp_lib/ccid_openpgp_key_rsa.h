/* 
 * File:   ccid_openpgp_key_rsa.h
 * Author: makmorit
 *
 * Created on 2021/02/18, 15:05
 */
#ifndef CCID_OPENPGP_KEY_RSA_H
#define CCID_OPENPGP_KEY_RSA_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
uint8_t    *ccid_openpgp_key_rsa_private_key(void);
uint8_t    *ccid_openpgp_key_rsa_public_key(void);
uint16_t    ccid_openpgp_key_rsa_nbits(uint8_t *key_attr, unsigned int *p_nbits);
uint16_t    ccid_openpgp_key_rsa_import(uint8_t *key_attr, uint8_t *privkey_pq);
uint16_t    ccid_openpgp_key_rsa_generate(uint8_t *key_attr);
uint16_t    ccid_openpgp_key_rsa_read(uint16_t key_tag);
uint16_t    ccid_openpgp_key_rsa_signature(uint16_t key_tag, uint8_t *key_attr, uint8_t *data, size_t size, uint8_t *signature, size_t *p_signature_size);
uint16_t    ccid_openpgp_key_rsa_decrypt(uint16_t key_tag, uint8_t *key_attr, uint8_t *encrypted, uint8_t *decrypted, size_t *p_decrypted_size);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OPENPGP_KEY_RSA_H */
