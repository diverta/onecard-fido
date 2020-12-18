//
//  tool_crypto_common.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/14.
//
#include <string.h>
#include <openssl/evp.h>

#include "debug_log.h"
#include "tool_crypto_common.h"

//
// public functions
//
unsigned char tool_crypto_get_algorithm_from_evp_pkey(void *_key)
{
    EVP_PKEY *key = (EVP_PKEY *)_key;
    int type = EVP_PKEY_base_id(key);
    int size = EVP_PKEY_bits(key);

    if (type == EVP_PKEY_RSA && size == RSA2048_N_SIZE) {
        return CRYPTO_ALG_RSA2048;
    }
    if (type == EVP_PKEY_EC && size == 256) {
        return CRYPTO_ALG_ECCP256;
    }
    log_debug("%s: Unsupported algorithm (type=0x%04x, size=%d)", __func__, type, size);
    return CRYPTO_ALG_NONE;
}
