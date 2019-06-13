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

static nrf_crypto_hash_context_t hash_context = {0};
static nrf_crypto_ecdsa_sign_context_t sign_context = {0};

// for generate random vector
#include "nrf_crypto_rng.h"
static uint8_t m_random_vector[64];

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

void fido_crypto_ecdsa_sign(nrf_crypto_ecc_private_key_t *private_key_for_sign, 
    uint8_t const *hash_digest, size_t digest_size, uint8_t *signature, size_t *signature_size)
{
    // Initialize crypto library.
    ret_code_t err_code = nrf_crypto_init();
    APP_ERROR_CHECK(err_code);

    err_code = nrf_crypto_ecdsa_sign(
        &sign_context, 
        private_key_for_sign,
        hash_digest,
        digest_size,
        signature, 
        signature_size);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("nrf_crypto_ecdsa_sign() returns 0x%02x ", err_code);
    }
    APP_ERROR_CHECK(err_code);
}
