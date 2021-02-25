/* 
 * File:   ccid_openpgp_crypto.h
 * Author: makmorit
 *
 * Created on 2021/02/23, 12:08
 */
#ifndef CCID_OPENPGP_CRYPTO_H
#define CCID_OPENPGP_CRYPTO_H

#include "ccid_apdu.h"

#ifdef __cplusplus
extern "C" {
#endif

uint16_t    ccid_openpgp_crypto_pso(command_apdu_t *capdu, response_apdu_t *rapdu);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OPENPGP_CRYPTO_H */
