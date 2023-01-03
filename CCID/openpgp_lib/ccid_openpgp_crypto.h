/* 
 * File:   ccid_openpgp_crypto.h
 * Author: makmorit
 *
 * Created on 2021/02/23, 12:08
 */
#ifndef CCID_OPENPGP_CRYPTO_H
#define CCID_OPENPGP_CRYPTO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t    ccid_openpgp_crypto_pso(void *p_capdu, void *p_rapdu);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OPENPGP_CRYPTO_H */
