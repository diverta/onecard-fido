/* 
 * File:   app_crypto_rsa.c
 * Author: makmorit
 *
 * Created on 2021/05/18, 15:01
 */
#include <zephyr/types.h>
#include <zephyr.h>

// for Mbed TLS
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/rsa.h>
#include <mbedtls/platform.h>

// ログ出力制御
#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_crypto_rsa);

// 定義体
#include "app_crypto.h"
#include "app_crypto_define.h"

//
// E は定数として、このモジュール内で管理
//
static uint8_t E[] = {0, 1, 0, 1};

uint8_t *app_crypto_rsa_e_bytes(void)
{
    return E;
}

uint8_t app_crypto_rsa_e_size(void)
{
    return sizeof(E);
}

//
// RSA-2048関連処理
//
static mbedtls_rsa_context rsa_context;

static bool app_crypto_rsa_private_terminate(bool success)
{
    // リソースを解放
    mbedtls_rsa_free(&rsa_context);
    return success;
}

static bool import_private_key_raw(uint8_t *rsa_private_key_raw)
{
    //
    // mbedtls_rsa_import_raw を実行
    // （P, Q, E のインポート）
    //
    uint8_t *p_P = rsa_private_key_raw;
    uint8_t *p_Q = p_P + RSA2048_PQ_SIZE;
    int ret = mbedtls_rsa_import_raw(&rsa_context, NULL, 0, p_P, RSA2048_PQ_SIZE, p_Q, RSA2048_PQ_SIZE, NULL, 0, E, sizeof(E));
    if (ret != 0) {
        LOG_ERR("mbedtls_rsa_import_raw returns %d", ret);
        return false;
    }

    //
    // mbedtls_rsa_complete を実行
    //
    ret = mbedtls_rsa_complete(&rsa_context);
    if (ret != 0) {
        LOG_ERR("mbedtls_rsa_complete returns %d", ret);
        return false;
    }

    return true;
}

bool app_crypto_rsa_private(uint8_t *rsa_private_key_raw, uint8_t *input, uint8_t *output)
{
    // 変数初期化
    mbedtls_rsa_init(&rsa_context, MBEDTLS_RSA_PKCS_V15, 0);

    // 引数領域の秘密鍵をインポート
    // （P、Q が連続して格納されている想定）
    if (import_private_key_raw(rsa_private_key_raw) == false) {
        return app_crypto_rsa_private_terminate(false);
    }

    //
    // mbedtls_rsa_private を実行
    //
    int ret = mbedtls_rsa_private(&rsa_context, &mbedtls_ctr_drbg_random, app_crypto_ctr_drbg_context(), input, output);
    if (ret != 0) {
        if (ret == (MBEDTLS_ERR_RSA_PRIVATE_FAILED + MBEDTLS_ERR_MPI_BAD_INPUT_DATA)) {
            LOG_ERR("Bad input data given ", ret);
        } else {
            LOG_ERR("mbedtls_rsa_private returns %d", ret);
        }
        return app_crypto_rsa_private_terminate(false);
    }

    // 正常終了
    return app_crypto_rsa_private_terminate(true);
}

bool app_crypto_rsa_public(uint8_t *rsa_public_key_raw, uint8_t *input, uint8_t *output)
{
    // 変数初期化
    mbedtls_rsa_init(&rsa_context, MBEDTLS_RSA_PKCS_V15, 0);

    //
    // mbedtls_rsa_import_raw を実行
    // （N, E のインポート）
    //
    uint8_t *p_N = rsa_public_key_raw;
    int ret = mbedtls_rsa_import_raw(&rsa_context, p_N, RSA2048_PQ_SIZE*2, NULL, 0, NULL, 0, NULL, 0, E, sizeof(E));
    if (ret != 0) {
        LOG_ERR("mbedtls_rsa_import_raw returns %d", ret);
        return ret;
    }

    //
    // mbedtls_rsa_public を実行
    //
    ret = mbedtls_rsa_public(&rsa_context, input, output);
    if (ret != 0) {
        if (ret == (MBEDTLS_ERR_RSA_PUBLIC_FAILED + MBEDTLS_ERR_MPI_BAD_INPUT_DATA)) {
            LOG_ERR("Bad input data given ", ret);
        } else {
            LOG_ERR("mbedtls_rsa_public returns %d", ret);
        }
        return app_crypto_rsa_private_terminate(false);
    }

    // 正常終了
    return app_crypto_rsa_private_terminate(true);
}

bool app_crypto_rsa_import_pubkey_from_prvkey(uint8_t *rsa_private_key_raw, uint8_t *rsa_public_key_raw)
{
    // 変数初期化
    mbedtls_rsa_init(&rsa_context, MBEDTLS_RSA_PKCS_V15, 0);

    // 引数領域の秘密鍵をインポート
    // （P、Q が連続して格納されている想定）
    if (import_private_key_raw(rsa_private_key_raw) == false) {
        return app_crypto_rsa_private_terminate(false);
    }

    //
    // mbedtls_rsa_export_raw を実行
    // （N のエクスポート）
    //
    uint8_t *n = rsa_public_key_raw;
    int ret = mbedtls_rsa_export_raw(&rsa_context, n, RSA2048_PQ_SIZE*2, NULL, 0, NULL, 0, NULL, 0, NULL, 0);
    if (ret != 0) {
        LOG_ERR("mbedtls_rsa_export_raw returns %d", ret);
        return app_crypto_rsa_private_terminate(false);
    }

    // 正常終了
    return app_crypto_rsa_private_terminate(true);
}
