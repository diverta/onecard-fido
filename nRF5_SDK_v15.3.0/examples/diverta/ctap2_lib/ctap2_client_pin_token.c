/* 
 * File:   ctap2_client_pin_token.c
 * Author: makmorit
 *
 * Created on 2019/02/23, 11:22
 */
#include "sdk_common.h"

#include <stdio.h>
#include <string.h>

// for nrf_drv_rng_xxx
#include "nrf_drv_rng.h"
#include "nrf_crypto_error.h"
#include "nrf_crypto_hmac.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ctap2_client_pin_token
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#include "fido_common.h"
#include "ctap2_common.h"
#include "fido_aes_cbc_256_crypto.h"

// PINトークン格納領域
#define PIN_TOKEN_SIZE 16
static uint8_t m_pin_token[PIN_TOKEN_SIZE];

// 暗号化されたPINトークンの格納領域
static size_t  encoded_pin_token_size;
static uint8_t encoded_pin_token[PIN_TOKEN_SIZE];

// PINトークンが生成済みかどうかを保持
static bool pin_token_generated = false;

// HMAC SHA-256ハッシュ格納領域
static nrf_crypto_hmac_context_t hmac_context;
static uint8_t                   hmac_data[NRF_CRYPTO_HASH_SIZE_SHA256];
static size_t                    hmac_data_size;

void ctap2_client_pin_token_init(bool force)
{
    // PINトークンが生成済みで、かつ
    // 強制再生成を要求しない場合は終了
    if (pin_token_generated && force == false) {
        NRF_LOG_DEBUG("PIN token is already exist");
        return;
    }

    // 16バイトのランダムベクターを生成
    memset(m_pin_token, 0, sizeof(m_pin_token));
    uint32_t err_code = nrf_drv_rng_rand(m_pin_token, PIN_TOKEN_SIZE);
    APP_ERROR_CHECK(err_code);

    // 生成済みフラグを設定
    if (!pin_token_generated) {
        NRF_LOG_DEBUG("PIN token generate success");
    } else {
        NRF_LOG_DEBUG("PIN token re-generate success");
    }
    pin_token_generated = true;
}

uint8_t *ctap2_client_pin_token_encoded(void)
{
    // PINトークンが未生成の場合は終了
    if (!pin_token_generated) {
        NRF_LOG_DEBUG("PIN token is not exist");
        return NULL;
    }

    // PINトークン格納領域の先頭アドレスを戻す
    return encoded_pin_token;
}

size_t ctap2_client_pin_token_encoded_size(void)
{
    // PINトークン長を戻す
    return PIN_TOKEN_SIZE;
}

uint8_t ctap2_client_pin_token_encode(uint8_t *p_key)
{
    // PINトークンを、共通鍵ハッシュを使用して復号化
    encoded_pin_token_size = fido_aes_cbc_256_encrypt(p_key, 
        m_pin_token, PIN_TOKEN_SIZE, encoded_pin_token);
    if (encoded_pin_token_size != PIN_TOKEN_SIZE) {
        // 処理NGの場合はエラーコードを戻す
        return CTAP1_ERR_OTHER;
    }

    return CTAP1_ERR_SUCCESS;
}

static void app_error_check(char *function, ret_code_t err_code)
{
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("%s returns 0x%04x(%s)", 
            function, err_code, nrf_crypto_error_string_get(err_code));
        APP_ERROR_CHECK(err_code);
    }
}

static uint8_t *calculate_hmac(
    uint8_t *key_data,   size_t key_size,
    uint8_t *src_data_1, size_t src_data_1_size)
{
    // HMACハッシュ計算には、引数の key_data を使用
    ret_code_t err_code = nrf_crypto_hmac_init(
        &hmac_context, &g_nrf_crypto_hmac_sha256_info, key_data, key_size);
    app_error_check("nrf_crypto_hmac_init", err_code);

    // 引数を計算対象に設定
    if (src_data_1 != NULL && src_data_1_size > 0) {
        err_code = nrf_crypto_hmac_update(&hmac_context, src_data_1, src_data_1_size);
        app_error_check("nrf_crypto_hmac_update", err_code);
    }

    // HMACハッシュを計算
    hmac_data_size = sizeof(hmac_data);
    err_code = nrf_crypto_hmac_finalize(&hmac_context, hmac_data, &hmac_data_size);
    app_error_check("nrf_crypto_hmac_finalize", err_code);

    // HMACハッシュの先頭アドレスを戻す
    return hmac_data;
}

uint8_t ctap2_client_pin_token_verify_pin_auth(uint8_t *clientDataHash, uint8_t *pinAuth)
{
    // clientDataHashからHMAC SHA-256ハッシュを計算
    uint8_t *p_hmac = calculate_hmac(m_pin_token, PIN_TOKEN_SIZE, clientDataHash, CLIENT_DATA_HASH_SIZE);

    // クライアントから受信したpinAuth（16バイト）を、
    // 生成されたHMAC SHA-256ハッシュと比較し、
    // 異なる場合はエラーを戻す
    if (memcmp(p_hmac, pinAuth, PIN_AUTH_SIZE) != 0) {
        return CTAP2_ERR_PIN_AUTH_INVALID;
    }

    return CTAP1_ERR_SUCCESS;
}
