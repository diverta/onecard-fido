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

// ランダムベクター格納領域
static uint8_t m_random_vector[64];

void fido_crypto_init(void)
{
    int ret = mbedtls_platform_setup(NULL);
    fido_log_info("Mbed TLS platform initialization returns %d", ret);
    if (ret != 0) {
        return;
    }
}

void _fido_crypto_generate_random_vector(uint8_t *vector_buf, size_t vector_buf_size)
{
    // Generate a random vector of specified length.
    memset(m_random_vector, 0, sizeof(m_random_vector));

    // 引数で指定のバイト数分、配列に格納
    for (uint8_t r = 0; r < vector_buf_size; r++) {
        vector_buf[r] = m_random_vector[r];
    }
}
