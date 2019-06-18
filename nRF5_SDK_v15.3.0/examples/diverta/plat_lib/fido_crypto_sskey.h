/* 
 * File:   fido_crypto_sskey.h
 * Author: makmorit
 *
 * Created on 2019/02/23, 11:17
 */
#ifndef FIDO_CRYPTO_SSKEY_H
#define FIDO_CRYPTO_SSKEY_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void     fido_crypto_sskey_init(bool force);
uint8_t  fido_crypto_sskey_generate(uint8_t *client_public_key_raw_data);
uint8_t *fido_crypto_sskey_public_key(void);
uint8_t *fido_crypto_sskey_hash(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_CRYPTO_SSKEY_H */

