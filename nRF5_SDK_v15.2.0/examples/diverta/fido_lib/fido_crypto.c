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
#include "app_error.h"

static nrf_crypto_hash_context_t hash_context = {0};

void fido_crypto_init(void)
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
    NRF_LOG_DEBUG("nrf_crypto_hash_calculate() returns 0x%02x ", err_code);
    APP_ERROR_CHECK(err_code);
}
