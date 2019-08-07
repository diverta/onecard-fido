/* 
 * File:   fido_crypto.cpp
 * Author: makmorit
 *
 * Created on 2019/08/07, 11:02
 */
#include "mbed.h"
#include "mbedtls/platform.h"

#include "fido_crypto.h"
#include "fido_log.h"

void fido_crypto_init(void)
{
    int ret = mbedtls_platform_setup(NULL);
    fido_log_info("Mbed TLS platform initialization returns %d", ret);
    if (ret != 0) {
        return;
    }
}
