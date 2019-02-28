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

size_t ctap2_client_pin_decrypt(uint8_t *p_key, uint8_t *p_encrypted, size_t encrypted_size, uint8_t *decrypted);

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_CLIENT_PIN_CRYPTO_H */
