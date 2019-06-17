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
#include "app_error.h"

// for calculate hmac
#include "nrf_crypto_hmac.h"

// for nrf_drv_rng_xxx
#include "nrf_drv_rng.h"
#include "nrf_crypto_error.h"

static nrf_crypto_hash_context_t hash_context = {0};
static nrf_crypto_ecdsa_sign_context_t sign_context = {0};

// 署名生成のための秘密鍵情報を保持
static nrf_crypto_ecc_private_key_t private_key_for_sign;

// for generate random vector
#include "nrf_crypto_rng.h"
static uint8_t m_random_vector[64];

// HMAC SHA-256ハッシュ格納領域
static nrf_crypto_hmac_context_t hmac_context;
static uint8_t                   hmac_data[NRF_CRYPTO_HASH_SIZE_SHA256];
static size_t                    hmac_data_size;

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

void fido_crypto_generate_sha256_hash(uint8_t *data, size_t data_size, nrf_crypto_hash_sha256_digest_t hash_digest, size_t *hash_digest_size)
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

void fido_crypto_ecdsa_sign(uint8_t *private_key_be, 
    uint8_t const *hash_digest, size_t digest_size, uint8_t *signature, size_t *signature_size)
{
    // Initialize crypto library.
    ret_code_t err_code = nrf_crypto_init();
    APP_ERROR_CHECK(err_code);

    // 署名に使用する秘密鍵（32バイト）を取得
    //   SDK 15以降はビッグエンディアンで引き渡す必要あり
    err_code = nrf_crypto_ecc_private_key_from_raw(
        &g_nrf_crypto_ecc_secp256r1_curve_info,
        &private_key_for_sign, 
        private_key_be, 
        NRF_CRYPTO_ECC_SECP256R1_RAW_PRIVATE_KEY_SIZE);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_DEBUG("nrf_crypto_ecc_private_key_from_raw() returns 0x%02x ", err_code);
    }
    APP_ERROR_CHECK(err_code);
    
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
    }
    APP_ERROR_CHECK(err_code);
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
