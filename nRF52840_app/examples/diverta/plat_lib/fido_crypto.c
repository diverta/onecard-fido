/* 
 * File:   fido_crypto.c
 * Author: makmorit
 *
 * Created on 2018/12/26, 12:19
 */
#include "sdk_common.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_crypto
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for generate hash
#include "nrf_crypto_init.h"
#include "nrf_crypto_hash.h"
#include "nrf_crypto_ecdsa.h"
#include "nrf_crypto_ecdh.h"
#include "app_error.h"

// for calculate hmac
#include "nrf_crypto_hmac.h"

// for mbed tls
#include "mbedtls/des.h"

static nrf_crypto_hash_context_t hash_context = {0};
static nrf_crypto_ecdsa_sign_context_t sign_context = {0};
static nrf_crypto_ecdsa_verify_context_t verify_context = {0};
static nrf_crypto_ecdh_context_t nrf_crypto_ecdh_context;

// 署名生成のための秘密鍵情報を保持
static nrf_crypto_ecc_private_key_t private_key_for_sign;

// 署名検証のための公開鍵情報を保持
static nrf_crypto_ecc_public_key_t  public_key_for_sign;

// for generate random vector
#include "nrf_crypto_rng.h"
static uint8_t m_random_vector[64];

// HMAC SHA-256ハッシュ格納領域
static nrf_crypto_hmac_context_t hmac_context;
static uint8_t                   hmac_data[NRF_CRYPTO_HASH_SIZE_SHA256];
static size_t                    hmac_data_size;

// SDK内部形式変換用の一時領域（公開鍵／秘密鍵）
static nrf_crypto_ecc_public_key_t client_public_key;
static nrf_crypto_ecc_private_key_t self_private_key;

void fido_crypto_init(void)
{
    ret_code_t err_code;
    if (nrf_crypto_is_initialized() == true) {
        return;
    }
    // 初期化が未実行の場合は
    // nrf_crypto_initを実行する
    err_code = nrf_crypto_init();
    if (err_code != NRF_ERROR_MODULE_ALREADY_INITIALIZED) {
        NRF_LOG_ERROR("nrf_crypto_init() returns 0x%02x ", err_code);
        APP_ERROR_CHECK(err_code);
    }
}

void fido_crypto_uninit(void)
{
    ret_code_t err_code;
    if (nrf_crypto_is_initialized() == true) {
        return;
    }
    // 初期化が実行ずみの場合は
    // nrf_crypto_deinitを実行する
    err_code = nrf_crypto_uninit();
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("nrf_crypto_uninit() returns 0x%02x ", err_code);
        APP_ERROR_CHECK(err_code);
    }
}

void fido_crypto_generate_sha256_hash(uint8_t *data, size_t data_size, uint8_t *hash_digest, size_t *hash_digest_size)
{    
    // nrf_cryptoの初期化を実行する
    fido_crypto_init();

    // 引数のバイト配列について、SHA-256ハッシュを生成
    ret_code_t err_code = nrf_crypto_hash_calculate(
        &hash_context, 
        &g_nrf_crypto_hash_sha256_info, 
        data, 
        data_size, 
        hash_digest, 
        hash_digest_size);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("nrf_crypto_hash_calculate() returns 0x%02x ", err_code);
    }
    APP_ERROR_CHECK(err_code);
}

void fido_crypto_generate_random_vector(uint8_t *vector_buf, size_t vector_buf_size)
{
    // Initialize crypto library.
    ret_code_t err_code = nrf_crypto_init();
    APP_ERROR_CHECK(err_code);

    // Generate a random vector of specified length.
    err_code = nrf_crypto_rng_vector_generate(m_random_vector, vector_buf_size);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("nrf_crypto_rng_vector_generate() returns 0x%02x ", err_code);
    }
    APP_ERROR_CHECK(err_code);

    // 引数で指定のバイト数分、配列に格納
    for (uint8_t r = 0; r < vector_buf_size; r++) {
        vector_buf[r] = m_random_vector[r];
    }
}

bool fido_crypto_ecdsa_sign(uint8_t *private_key_be, 
    uint8_t const *hash_digest, size_t digest_size, uint8_t *signature, size_t *signature_size)
{
    // Initialize crypto library.
    ret_code_t err_code = nrf_crypto_init();
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_DEBUG("nrf_crypto_init() returns 0x%02x ", err_code);
        return false;
    }

    // 署名に使用する秘密鍵（32バイト）を取得
    //   SDK 15以降はビッグエンディアンで引き渡す必要あり
    err_code = nrf_crypto_ecc_private_key_from_raw(
        &g_nrf_crypto_ecc_secp256r1_curve_info,
        &private_key_for_sign, 
        private_key_be, 
        NRF_CRYPTO_ECC_SECP256R1_RAW_PRIVATE_KEY_SIZE);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_DEBUG("nrf_crypto_ecc_private_key_from_raw() returns 0x%02x ", err_code);
        return false;
    }
    
    // 署名実行
    err_code = nrf_crypto_ecdsa_sign(
        &sign_context, 
        &private_key_for_sign,
        hash_digest,
        digest_size,
        signature, 
        signature_size);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("nrf_crypto_ecdsa_sign() returns 0x%02x ", err_code);
        return false;
    }

    return true;
}

bool fido_crypto_ecdsa_sign_verify(uint8_t *public_key_be, 
    uint8_t const *hash_digest, size_t digest_size, uint8_t *signature, size_t signature_size)
{
    // Initialize crypto library.
    ret_code_t err_code = nrf_crypto_init();
    APP_ERROR_CHECK(err_code);

    // 検証に使用する公開鍵（64バイト）を取得
    //   SDK 15以降はビッグエンディアンで引き渡す必要あり
    err_code = nrf_crypto_ecc_public_key_from_raw(
        &g_nrf_crypto_ecc_secp256r1_curve_info,
        &public_key_for_sign, 
        public_key_be, 
        NRF_CRYPTO_ECC_SECP256R1_RAW_PUBLIC_KEY_SIZE);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_DEBUG("nrf_crypto_ecc_public_key_from_raw() returns 0x%02x ", err_code);
    }
    APP_ERROR_CHECK(err_code);

    // 署名検証実行
    err_code = nrf_crypto_ecdsa_verify(
        &verify_context, 
        &public_key_for_sign, 
        hash_digest,
        digest_size,
        signature, 
        signature_size);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("nrf_crypto_ecdsa_verify() returns 0x%02x ", err_code);
        return false;
    }
    return true;
}

void fido_crypto_calculate_hmac_sha256(
    uint8_t *key_data, size_t key_data_size, 
    uint8_t *src_data, size_t src_data_size, uint8_t *src_data_2, size_t src_data_2_size,
    uint8_t *dest_data)
{
    // HMACハッシュ計算には、引数のkey_dataを使用
    ret_code_t err_code = nrf_crypto_hmac_init(
        &hmac_context, &g_nrf_crypto_hmac_sha256_info, 
        key_data, key_data_size);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("nrf_crypto_hmac_init failed");
    }
    APP_ERROR_CHECK(err_code);

    // 引数を計算対象に設定
    err_code = nrf_crypto_hmac_update(&hmac_context, src_data, src_data_size);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("nrf_crypto_hmac_update failed");
    }
    APP_ERROR_CHECK(err_code);

    // 2番目の引数を計算対象に設定
    if (src_data_2 != NULL && src_data_2_size > 0) {
        err_code = nrf_crypto_hmac_update(&hmac_context, src_data_2, src_data_2_size);
        if (err_code != NRF_SUCCESS) {
            NRF_LOG_ERROR("nrf_crypto_hmac_update failed");
        }
        APP_ERROR_CHECK(err_code);
    }

    // HMACハッシュを計算
    hmac_data_size = sizeof(hmac_data);
    err_code = nrf_crypto_hmac_finalize(&hmac_context, hmac_data, &hmac_data_size);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("nrf_crypto_hmac_finalize failed");
    }
    APP_ERROR_CHECK(err_code);

    // 計算結果をdestDataへコピー
    for (uint8_t i = 0; i < sizeof(hmac_data); i++) {
        dest_data[i] = hmac_data[i];
    }
}

bool fido_crypto_calculate_ecdh(uint8_t *private_key_raw_data, uint8_t *client_public_key_raw_data, uint8_t *sskey_raw_data, size_t *sskey_raw_data_size)
{
    // Initialize crypto library.
    ret_code_t err_code = nrf_crypto_init();
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("nrf_crypto_init returns 0x%04x(%s)", 
            err_code, nrf_crypto_error_string_get(err_code));
        return false;
    }
    NRF_LOG_DEBUG("Compute shared secret using ECDH start");

    // 公開鍵をSDK内部形式に変換
    err_code = nrf_crypto_ecc_public_key_from_raw(
        &g_nrf_crypto_ecc_secp256r1_curve_info, 
        &client_public_key, client_public_key_raw_data, 
        NRF_CRYPTO_ECC_SECP256R1_RAW_PUBLIC_KEY_SIZE);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("nrf_crypto_ecc_public_key_from_raw returns 0x%04x(%s)", 
            err_code, nrf_crypto_error_string_get(err_code));
        return false;
    }

    // 秘密鍵をSDK内部形式に変換
    err_code = nrf_crypto_ecc_private_key_from_raw(
        &g_nrf_crypto_ecc_secp256r1_curve_info, 
        &self_private_key, private_key_raw_data, 
        NRF_CRYPTO_ECC_SECP256R1_RAW_PRIVATE_KEY_SIZE);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("nrf_crypto_ecc_private_key_from_raw returns 0x%04x(%s)", 
            err_code, nrf_crypto_error_string_get(err_code));
        return false;
    }

    // 共通鍵を生成
    *sskey_raw_data_size = NRF_CRYPTO_ECDH_SECP256R1_SHARED_SECRET_SIZE;
    err_code = nrf_crypto_ecdh_compute(&nrf_crypto_ecdh_context,
        &self_private_key, &client_public_key, sskey_raw_data, sskey_raw_data_size);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("nrf_crypto_ecdh_compute returns 0x%04x(%s)", 
            err_code, nrf_crypto_error_string_get(err_code));
        return false;
    }

    NRF_LOG_DEBUG("Compute shared secret using ECDH end");
    return true;
}

bool fido_crypto_tdes_enc(const uint8_t *in, uint8_t *out, const uint8_t *key)
{
    mbedtls_des3_context ctx;
    mbedtls_des3_init(&ctx);
    mbedtls_des3_set3key_enc(&ctx, key);

    int ret = mbedtls_des3_crypt_ecb(&ctx, in, out);
    if (ret < 0) {
        NRF_LOG_ERROR("mbedtls_des3_crypt_ecb returns %d", ret);
        return false;
    }

    mbedtls_des3_free(&ctx);
    return true;
}
