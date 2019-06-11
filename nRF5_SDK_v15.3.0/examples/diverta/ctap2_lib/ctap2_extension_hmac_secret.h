/* 
 * File:   ctap2_extension_hmac_secret.h
 * Author: makmorit
 *
 * Created on 2019/05/20, 10:30
 */
#ifndef CTAP2_EXTENSION_HMAC_SECRET_H
#define CTAP2_EXTENSION_HMAC_SECRET_H

#include "ctap2_common.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t *ctap2_extension_hmac_secret_cbor(void);
size_t   ctap2_extension_hmac_secret_cbor_size(void);
uint8_t  ctap2_extension_hmac_secret_cbor_for_create(void);
uint8_t  ctap2_extension_hmac_secret_cbor_for_get(CTAP_EXTENSIONS_T *ext);

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_EXTENSION_HMAC_SECRET_H */

