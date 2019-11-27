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

bool fido_cryptoauth_init(void);
void fido_cryptoauth_release(void);

void     fido_cryptoauth_keypair_generate(uint16_t key_id);
uint8_t *fido_cryptoauth_keypair_public_key(uint16_t key_id);
size_t   fido_cryptoauth_keypair_private_key_size(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_CRYPTOAUTH_H */
