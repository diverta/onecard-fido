/* 
 * File:   fido_crypto.h
 * Author: makmorit
 *
 * Created on 2019/08/07, 11:02
 */
#ifndef FIDO_CRYPTO_H
#define FIDO_CRYPTO_H

#ifdef __cplusplus
extern "C" {
#endif

void fido_crypto_init(void);

//
// C --> CPP 呼出用インターフェース
//
void _fido_crypto_generate_random_vector(uint8_t *vector_buf, size_t vector_buf_size);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_CRYPTO_H */
