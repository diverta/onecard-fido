/* 
 * File:   fido_crypto_ecb.h
 * Author: makmorit
 *
 * Created on 2018/12/27, 14:48
 */
#ifndef FIDO_CRYPTO_ECB_H
#define FIDO_CRYPTO_ECB_H

#ifdef __cplusplus
extern "C" {
#endif

bool fido_crypto_ecb_init(void);
void fido_crypto_ecb_encrypt(uint8_t *packet, uint32_t packet_length, uint8_t *out_packet);
void fido_crypto_ecb_decrypt(uint8_t *packet, uint32_t packet_length, uint8_t *out_packet);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_CRYPTO_ECB_H */
