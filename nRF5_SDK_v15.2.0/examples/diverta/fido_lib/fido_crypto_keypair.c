/* 
 * File:   fido_crypto_keypair.c
 * Author: makmorit
 *
 * Created on 2018/12/27, 11:39
 */
#include "sdk_common.h"

#include <stdio.h>
#include <string.h>

// for keysize informations
#include "nrf_crypto_init.h"
#include "nrf_crypto_ecdsa.h"
#include "app_error.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_crypto_keypair
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// nrf_cc310で生成される鍵情報
static nrf_crypto_ecc_key_pair_generate_context_t keygen_context;
static nrf_crypto_ecc_private_key_t               private_key;
static nrf_crypto_ecc_public_key_t                public_key;

// 鍵ペア情報をRAWデータに変換する領域
//   この領域に格納される鍵は
//   ビッグエンディアン配列となる
static uint8_t private_key_raw_data[NRF_CRYPTO_ECC_SECP256R1_RAW_PRIVATE_KEY_SIZE];
static uint8_t public_key_raw_data[NRF_CRYPTO_ECC_SECP256R1_RAW_PUBLIC_KEY_SIZE];
static size_t  private_key_raw_data_size;
static size_t  public_key_raw_data_size;

static void fido_crypto_init(void)
{
    ret_code_t err_code;
    if (nrf_crypto_is_initialized() == true) {
        return;
    }
    // 初期化が未実行の場合は
    // nrf_crypto_initを実行する
    err_code = nrf_crypto_init();
    NRF_LOG_DEBUG("nrf_crypto_init() returns 0x%02x ", err_code);
    if (err_code != NRF_ERROR_MODULE_ALREADY_INITIALIZED) {
        APP_ERROR_CHECK(err_code);
    }
}

static void generate_keypair(void)
{
    ret_code_t err_code;

    // nrf_cryptoの初期化を実行する
    fido_crypto_init();

    // 秘密鍵および公開鍵を生成する
    err_code = nrf_crypto_ecc_key_pair_generate(
        &keygen_context, &g_nrf_crypto_ecc_secp256r1_curve_info, &private_key, &public_key);
    NRF_LOG_DEBUG("nrf_crypto_ecc_key_pair_generate() returns 0x%02x ", err_code);
    APP_ERROR_CHECK(err_code);
}

static void convert_private_key_to_be(uint8_t *p_raw_data, size_t *p_raw_data_size)
{
    // 秘密鍵データをビッグエンディアンでp_raw_data配列に格納
    *p_raw_data_size = NRF_CRYPTO_ECC_SECP256R1_RAW_PRIVATE_KEY_SIZE;
    ret_code_t err_code = nrf_crypto_ecc_private_key_to_raw(&private_key, p_raw_data, p_raw_data_size);
    NRF_LOG_DEBUG("nrf_crypto_ecc_private_key_to_raw() returns 0x%02x ", err_code);
    APP_ERROR_CHECK(err_code);
}

static void convert_public_key_to_be(uint8_t *p_raw_data, size_t *p_raw_data_size)
{
    // 公開鍵データをビッグエンディアンでp_raw_data配列に格納
    *p_raw_data_size = NRF_CRYPTO_ECC_SECP256R1_RAW_PUBLIC_KEY_SIZE;
    ret_code_t err_code = nrf_crypto_ecc_public_key_to_raw(&public_key, p_raw_data, p_raw_data_size);
    NRF_LOG_DEBUG("nrf_crypto_ecc_public_key_to_raw() returns 0x%02x ", err_code);
    APP_ERROR_CHECK(err_code);
}

void fido_crypto_keypair_generate(void)
{
    // nrf_cc310により、キーペアを新規生成する
    generate_keypair();

    // 生成されたキーペアをビッグエンディアンに変換
    convert_private_key_to_be(private_key_raw_data, &private_key_raw_data_size);
    convert_public_key_to_be(public_key_raw_data, &public_key_raw_data_size);
}

uint8_t *fido_crypto_keypair_private_key(void)
{
    return private_key_raw_data;
}

uint8_t *fido_crypto_keypair_public_key(void)
{
    return public_key_raw_data;
}

size_t fido_crypto_keypair_private_key_size(void)
{
    return private_key_raw_data_size;
}

size_t fido_crypto_keypair_public_key_size(void)
{
    return public_key_raw_data_size;
}
