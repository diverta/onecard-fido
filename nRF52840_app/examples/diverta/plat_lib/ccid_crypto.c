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

// テスト用
#define LOG_DEBUG_RSA_EXPONENT      false

#include "ccid.h"
#include "ccid_piv_object.h"

// for nrf_cc310
#include "fido_crypto_plat.h"

// for mbedtls_rsa_private
#include "mbedtls/rsa.h"

// for fido_get_uint32_from_bytes
#include "fido_common.h"

//
// E は定数として、このモジュール内で管理
//
static uint8_t E[] = {0, 1, 0, 1};

uint8_t *ccid_crypto_rsa_e_bytes(void)
{
    return E;
}

uint8_t ccid_crypto_rsa_e_size(void)
{
    return sizeof(E);
}

static int ccid_crypto_rsa_exponent(void)
{
    int int_e = (int)fido_get_uint32_from_bytes(E);

#if LOG_DEBUG_RSA_EXPONENT
    NRF_LOG_DEBUG("ccid_crypto_rsa_exponent returns %d", int_e);
#endif

    return int_e;
}

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

static int import_private_key_raw(mbedtls_rsa_context *rsa, uint8_t *rsa_private_key_raw)
{
    //
    // mbedtls_rsa_import_raw を実行
    // （P, Q, E のインポート）
    //
    uint8_t *p_P = rsa_private_key_raw;
    uint8_t *p_Q = p_P + RSA2048_PQ_LENGTH;
    int ret = mbedtls_rsa_import_raw(rsa, NULL, 0, p_P, RSA2048_PQ_LENGTH, p_Q, RSA2048_PQ_LENGTH, NULL, 0, E, sizeof(E));
    if (ret != 0) {
        NRF_LOG_ERROR("mbedtls_rsa_import_raw returns %d", ret);
        return ret;
    }

    //
    // mbedtls_rsa_complete を実行
    //
    ret = mbedtls_rsa_complete(rsa);
    if (ret != 0) {
        NRF_LOG_ERROR("mbedtls_rsa_complete returns %d", ret);
        return ret;
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

    // 引数領域の秘密鍵をインポート
    // （P、Q が連続して格納されている想定）
    int ret = import_private_key_raw(&rsa, rsa_private_key_raw);
    if (ret != 0) {
        return ccid_crypto_rsa_private_terminate(false, &rsa);
    }

    //
    // mbedtls_rsa_private を実行
    //
    ret = mbedtls_rsa_private(&rsa, ccid_crypto_rsa_random, NULL, input, output);
    if (ret != 0) {
        NRF_LOG_ERROR("mbedtls_rsa_private returns %d", ret);
        return ccid_crypto_rsa_private_terminate(false, &rsa);
    }

    // 正常終了
    return ccid_crypto_rsa_private_terminate(true, &rsa);
}

bool ccid_crypto_rsa_import(uint8_t *rsa_private_key_raw, uint8_t *rsa_public_key_raw, unsigned int nbits)
{
    // nrf_cc310 を初期化
    fido_crypto_init();

    // 変数初期化
    mbedtls_rsa_context rsa;
    mbedtls_rsa_init(&rsa, MBEDTLS_RSA_PKCS_V15, 0);

    // 引数領域の秘密鍵をインポート
    // （P、Q が連続して格納されている想定）
    int ret = import_private_key_raw(&rsa, rsa_private_key_raw);
    if (ret != 0) {
        return ccid_crypto_rsa_private_terminate(false, &rsa);
    }

    //
    // mbedtls_rsa_export_raw を実行
    // （N のエクスポート）
    //
    size_t pq_size = nbits / 16;
    uint8_t *n = rsa_public_key_raw;
    ret = mbedtls_rsa_export_raw(&rsa, n, pq_size * 2, NULL, 0, NULL, 0, NULL, 0, NULL, 0);
    if (ret != 0) {
        NRF_LOG_ERROR("mbedtls_rsa_export_raw returns %d", ret);
        return ccid_crypto_rsa_private_terminate(false, &rsa);
    }

    // 正常終了
    return ccid_crypto_rsa_private_terminate(true, &rsa);
}

//
// RSA秘密鍵／公開鍵生成関連
//
static bool rsa_generate_key_terminate(bool success, mbedtls_rsa_context *rsa)
{
    // リソースを解放
    mbedtls_rsa_free(rsa);
    fido_crypto_uninit();
    return success;
}

bool ccid_crypto_rsa_generate_key(uint8_t *rsa_private_key_raw, uint8_t *rsa_public_key_raw, unsigned int nbits)
{
    // nrf_cc310 を初期化
    fido_crypto_init();

    // 変数初期化
    mbedtls_rsa_context rsa;
    mbedtls_rsa_init(&rsa, MBEDTLS_RSA_PKCS_V15, 0);

    //
    // mbedtls_rsa_gen_key を実行
    //   exponent = 65537 (0x00010001)
    //
    int ret = mbedtls_rsa_gen_key(&rsa, ccid_crypto_rsa_random, NULL, nbits, ccid_crypto_rsa_exponent());
    if (ret != 0) {
        NRF_LOG_ERROR("mbedtls_rsa_gen_key returns %d", ret);
        return rsa_generate_key_terminate(false, &rsa);
    }

    //
    // mbedtls_rsa_export_raw を実行
    // （N, P, Q のエクスポート）
    // offset of rsa_private_key_raw
    //    0: P
    //  128: Q
    //
    size_t pq_size = nbits / 16;
    uint8_t *n = rsa_public_key_raw;
    uint8_t *p = rsa_private_key_raw;
    uint8_t *q = p + pq_size;
    ret = mbedtls_rsa_export_raw(&rsa, n, pq_size * 2, p, pq_size, q, pq_size, NULL, 0, NULL, 0);
    if (ret != 0) {
        NRF_LOG_ERROR("mbedtls_rsa_export_raw returns %d", ret);
        return rsa_generate_key_terminate(false, &rsa);
    }

    // 正常終了
    return rsa_generate_key_terminate(true, &rsa);
}
