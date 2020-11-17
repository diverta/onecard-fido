/* 
 * File:   ccid_crypto.c
 * Author: makmorit
 *
 * Created on 2020/11/12, 9:32
 */
#include "sdk_common.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ccid_crypto
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#include "ccid.h"
#include "ccid_piv_object.h"

// for nrf_cc310
#include "fido_crypto.h"

// for mbedtls_rsa_private
#include "mbedtls/rsa.h"

static bool ccid_crypto_rsa_private_terminate(bool success, mbedtls_rsa_context *rsa)
{
    // リソースを解放
    mbedtls_rsa_free(rsa);
    return success;
}

static int ccid_crypto_rsa_random(void *ctx, unsigned char *buf, size_t size)
{
    (void)ctx;
    static uint32_t seed = 0;

    // 指定バイト数のランダムベクターを生成
    uint32_t r = 0;
    for (size_t i = 0; i < size; i++) {
        if (i % 4 == 0) {
            seed = 0x19660D * seed + 0x3C6EF35F;
            r = seed;
        }
        buf[i] = (r >> ((i % 4) * 8)) & 0xFF;
    }
    return 0;
}

bool ccid_crypto_rsa_private(uint8_t *rsa_private_key_raw, uint8_t *input, uint8_t *output)
{
    // nrf_cc310 を初期化
    fido_crypto_init();

    // 変数初期化
    mbedtls_rsa_context rsa;
    mbedtls_rsa_init(&rsa, MBEDTLS_RSA_PKCS_V15, 0);
    int ret;

    //
    // mbedtls_rsa_import_raw と等価の処理を実行
    // （P, Q, E のインポート）
    //
    uint8_t *p_P = rsa_private_key_raw;
    ret = mbedtls_mpi_read_binary(&rsa.P, p_P, RSA2048_PQ_LENGTH);
    if (ret != 0) {
        NRF_LOG_ERROR("mbedtls_mpi_read_binary(P) returns 0x%04x", ret);
        return ccid_crypto_rsa_private_terminate(false, &rsa);
    }
    uint8_t *p_Q = p_P + RSA2048_PQ_LENGTH;
    ret = mbedtls_mpi_read_binary(&rsa.Q, p_Q, RSA2048_PQ_LENGTH);
    if (ret != 0) {
        NRF_LOG_ERROR("mbedtls_mpi_read_binary(Q) returns 0x%04x", ret);
        return ccid_crypto_rsa_private_terminate(false, &rsa);
    }
    uint8_t E[] = {0, 1, 0, 1};
    ret = mbedtls_mpi_read_binary(&rsa.E, E, sizeof(E));
    if (ret != 0) {
        NRF_LOG_ERROR("mbedtls_mpi_read_binary(E) returns 0x%04x", ret);
        return ccid_crypto_rsa_private_terminate(false, &rsa);
    }

    //
    // mbedtls_rsa_complete を実行
    //
    ret = mbedtls_rsa_complete(&rsa);
    if (ret != 0) {
        NRF_LOG_ERROR("mbedtls_rsa_complete returns 0x%04x", ret);
        return ccid_crypto_rsa_private_terminate(false, &rsa);
    }

    //
    // mbedtls_rsa_private を実行
    //
    ret = mbedtls_rsa_private(&rsa, ccid_crypto_rsa_random, NULL, input, output);
    if (ret != 0) {
        NRF_LOG_ERROR("mbedtls_rsa_private returns 0x%04x", ret);
        return ccid_crypto_rsa_private_terminate(false, &rsa);
    }

    // 正常終了
    return ccid_crypto_rsa_private_terminate(true, &rsa);
}
