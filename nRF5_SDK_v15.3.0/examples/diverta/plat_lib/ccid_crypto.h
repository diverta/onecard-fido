/* 
 * File:   ccid_crypto.h
 * Author: makmorit
 *
 * Created on 2020/11/12, 9:32
 */
#ifndef CCID_CRYPTO_H
#define CCID_CRYPTO_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool ccid_crypto_rsa_private(uint8_t *rsa_private_key_raw, uint8_t *input, uint8_t *output);

#ifdef __cplusplus
}
#endif

#endif /* CCID_CRYPTO_H */
