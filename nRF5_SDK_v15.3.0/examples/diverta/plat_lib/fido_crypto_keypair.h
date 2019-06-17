/* 
 * File:   fido_crypto_keypair.h
 * Author: makmorit
 *
 * Created on 2018/12/27, 11:39
 */
#ifndef FIDO_CRYPTO_KEYPAIR_H
#define FIDO_CRYPTO_KEYPAIR_H

#ifdef __cplusplus
extern "C" {
#endif

void     fido_crypto_keypair_generate(void);
uint8_t *fido_crypto_keypair_private_key(void);
uint8_t *fido_crypto_keypair_public_key(void);
size_t   fido_crypto_keypair_private_key_size(void);
size_t   fido_crypto_keypair_public_key_size(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_CRYPTO_KEYPAIR_H */
