/* 
 * File:   ctap2_client_pin_crypto.h
 * Author: makmorit
 *
 * Created on 2019/02/25, 15:11
 */
#ifndef CTAP2_CLIENT_PIN_CRYPTO_H
#define CTAP2_CLIENT_PIN_CRYPTO_H

#ifdef __cplusplus
extern "C" {
#endif

bool ctap2_client_pin_crypto_init(uint8_t *p_password);
void ctap2_client_pin_decrypt(uint8_t *p_ciphertext, size_t ciphertext_size, uint8_t *out_packet);

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_CLIENT_PIN_CRYPTO_H */
