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

// for logging informations
#define NRF_LOG_MODULE_NAME ctap2_client_pin_token
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#include "fido_common.h"
#include "ctap2_client_pin_crypto.h"

// PINトークン格納領域
#define PIN_TOKEN_SIZE 16
static uint8_t m_pin_token[PIN_TOKEN_SIZE];

// 暗号化されたPINトークンの格納領域
static size_t  encoded_pin_token_size;
static uint8_t encoded_pin_token[PIN_TOKEN_SIZE];

// PINトークンが生成済みかどうかを保持
static bool pin_token_generated = false;

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
    encoded_pin_token_size = ctap2_client_pin_encrypt(p_key, 
        m_pin_token, PIN_TOKEN_SIZE, encoded_pin_token);
    if (encoded_pin_token_size != PIN_TOKEN_SIZE) {
        // 処理NGの場合はエラーコードを戻す
        return CTAP1_ERR_OTHER;
    }

    return CTAP1_ERR_SUCCESS;
}
