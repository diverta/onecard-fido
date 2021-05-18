/* 
 * File:   app_crypto_ec.c
 * Author: makmorit
 *
 * Created on 2021/05/17, 12:06
 */
#include <zephyr/types.h>
#include <zephyr.h>

// for Mbed TLS
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/platform.h>

// ログ出力制御
#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_crypto_ec);

// 定義体
#include "app_crypto.h"
#include "app_crypto_define.h"

//
// ECDSA署名処理
//
static mbedtls_ecdsa_context ecdsa_context;
static mbedtls_mpi r;
static mbedtls_mpi s;

static bool dsa_sign_terminate(bool b)
{
    // Free resources
    mbedtls_mpi_free(&r);
    mbedtls_mpi_free(&s);
    mbedtls_ecdsa_free(&ecdsa_context);
    return b;
}

bool app_crypto_ec_dsa_sign(uint8_t *private_key_be, uint8_t const *hash_digest, size_t digest_size, uint8_t *signature)
{
    // Initialize ECDSA context
    mbedtls_ecdsa_init(&ecdsa_context);
    int ret = mbedtls_ecp_group_load(&ecdsa_context.grp, MBEDTLS_ECP_DP_SECP256R1);
    if (ret != 0) {
        LOG_ERR("mbedtls_ecp_group_load returns %d", ret);
        return dsa_sign_terminate(false);
    }

    // 署名に使用する秘密鍵（32バイト）を取得
    ret = mbedtls_mpi_read_binary(&ecdsa_context.d, private_key_be, 32);
    if (ret != 0) {
        LOG_ERR("mbedtls_mpi_read_binary returns %d", ret);
        return dsa_sign_terminate(false);
    }

    // 署名格納用の領域を初期化
    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);

    // 署名実行
    ret = mbedtls_ecdsa_sign(&ecdsa_context.grp, &r, &s, &ecdsa_context.d, hash_digest, digest_size, &mbedtls_ctr_drbg_random, app_crypto_ctr_drbg_context());
    if (ret != 0) {
        LOG_ERR("mbedtls_ecdsa_sign returns %d", ret);
        return dsa_sign_terminate(false);
    }

    // 署名データをビッグエンディアンでバッファにコピー
    ret = mbedtls_mpi_write_binary(&r, signature, 32);
    if (ret != 0) {
        LOG_ERR("mbedtls_mpi_write_binary(R) returns %d", ret);
        return dsa_sign_terminate(false);
    }
    ret = mbedtls_mpi_write_binary(&s, signature + 32, 32);
    if (ret != 0) {
        LOG_ERR("mbedtls_mpi_write_binary(S) returns %d", ret);
        return dsa_sign_terminate(false);
    }

    return dsa_sign_terminate(true);
}

bool app_crypto_ec_dsa_verify(uint8_t *public_key_be, uint8_t const *hash_digest, size_t digest_size, uint8_t *signature)
{
    // Initialize ECDSA context
    mbedtls_ecdsa_init(&ecdsa_context);
    int ret = mbedtls_ecp_group_load(&ecdsa_context.grp, MBEDTLS_ECP_DP_SECP256R1);
    if (ret != 0) {
        LOG_ERR("mbedtls_ecp_group_load returns %d", ret);
        return dsa_sign_terminate(false);
    }

    // 公開鍵のバイナリーを読込み
    // （最初の１バイトが 0x04 で始まることが前提）
    ret = mbedtls_ecp_point_read_binary(&ecdsa_context.grp, &ecdsa_context.Q, public_key_be, 65);
    if (ret != 0) {
        LOG_ERR("mbedtls_ecp_point_read_binary returns %d", ret);
        return dsa_sign_terminate(false);
    }

    // 署名格納用の領域を初期化
    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);

    // 署名のバイナリーを読込み
    ret = mbedtls_mpi_read_binary(&r, signature, 32);
    if (ret != 0) {
        LOG_ERR("mbedtls_mpi_read_binary(R) returns %d", ret);
        return dsa_sign_terminate(false);
    }
    ret = mbedtls_mpi_read_binary(&s, signature + 32, 32);
    if (ret != 0) {
        LOG_ERR("mbedtls_mpi_read_binary(S) returns %d", ret);
        return dsa_sign_terminate(false);
    }

    // 署名の検証
    ret = mbedtls_ecdsa_verify(&ecdsa_context.grp, hash_digest, digest_size, &ecdsa_context.Q, &r, &s);
    if (ret != 0) {
        LOG_ERR("mbedtls_ecdsa_verify returns %d", ret);
        return dsa_sign_terminate(false);
    }    

    return dsa_sign_terminate(true);
}

//
// EC鍵ペア生成
//
static mbedtls_ecp_keypair ecp_keypair;

static bool keypair_generate_terminate(bool b)
{
    // Free resources
    mbedtls_ecp_keypair_free(&ecp_keypair);
    return b;
}

bool app_crypto_ec_keypair_generate(uint8_t *private_key_raw_data, uint8_t *public_key_raw_data)
{
    // キーペアを新規生成する
    mbedtls_ecp_keypair_init(&ecp_keypair);
    int ret = mbedtls_ecp_gen_key(MBEDTLS_ECP_DP_SECP256R1, &ecp_keypair, &mbedtls_ctr_drbg_random, app_crypto_ctr_drbg_context());
    if (ret != 0) {
        LOG_ERR("mbedtls_ecp_gen_key returns %d", ret);
        return keypair_generate_terminate(false);
    }

    // 生成されたキーペアをビッグエンディアンでバッファにコピー
    ret = mbedtls_mpi_write_binary(&ecp_keypair.d, private_key_raw_data, 32);
    if (ret != 0) {
        LOG_ERR("mbedtls_mpi_write_binary returns %d", ret);
        return keypair_generate_terminate(false);
    }
    size_t size;
    ret = mbedtls_ecp_point_write_binary(&ecp_keypair.grp, &ecp_keypair.Q, MBEDTLS_ECP_PF_UNCOMPRESSED, &size, public_key_raw_data, 65);
    if (ret != 0) {
        LOG_ERR("mbedtls_ecp_point_write_binary returns %d", ret);
        return keypair_generate_terminate(false);
    }

    return keypair_generate_terminate(true);
}
