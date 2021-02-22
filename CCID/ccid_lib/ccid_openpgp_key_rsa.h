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
uint16_t    ccid_openpgp_key_rsa_generate(uint8_t *key_attr);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OPENPGP_KEY_RSA_H */
