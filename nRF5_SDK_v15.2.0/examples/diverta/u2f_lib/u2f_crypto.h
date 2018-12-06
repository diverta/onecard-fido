#ifndef U2F_CRYPTO_H__
#define U2F_CRYPTO_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


uint8_t *u2f_crypto_signature_data_buffer(void);
size_t   u2f_crypto_signature_data_size(void);
void     u2f_crypto_signature_data_size_set(size_t size);
void     u2f_crypto_init(void);
void     u2f_crypto_generate_keypair(void);
uint32_t u2f_crypto_sign(uint8_t *private_key_be);
bool     u2f_crypto_create_asn1_signature(void);

void     u2f_crypto_private_key(uint8_t *p_raw_data, size_t *p_raw_data_size);
void     u2f_crypto_public_key(uint8_t *p_raw_data, size_t *p_raw_data_size);


#ifdef __cplusplus
}
#endif

#endif // U2F_CRYPTO_H__

/** @} */
