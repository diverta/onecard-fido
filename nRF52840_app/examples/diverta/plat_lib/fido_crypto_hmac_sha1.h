/* 
 * File:   fido_crypto_hmac_sha1.h
 * Author: makmorit
 *
 * Created on 2022/11/24, 14:44
 */
#ifndef FIDO_CRYPTO_HMAC_SHA1_H
#define FIDO_CRYPTO_HMAC_SHA1_H

#ifdef __cplusplus
extern "C" {
#endif

void fido_crypto_hmac_sha1_calculate(uint8_t *key_data, size_t key_data_size, uint8_t *src_data, size_t src_data_size, uint8_t *src_data_2, size_t src_data_2_size, uint8_t *dest_data);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_CRYPTO_HMAC_SHA1_H */
