/* 
 * File:   ctap2_client_pin_token.c
 * Author: makmorit
 *
 * Created on 2019/02/23, 11:22
 */
#include <stdio.h>
#include <string.h>

#include "fido_common.h"
#include "ctap2_common.h"
#include "fido_aes_cbc_256_crypto.h"
#include "fido_crypto.h"
#include "fido_log.h"

// PINトークン格納領域
#define PIN_TOKEN_SIZE 16
static uint8_t m_pin_token[PIN_TOKEN_SIZE];

// 暗号化されたPINトークンの格納領域
static size_t  encoded_pin_token_size;
static uint8_t encoded_pin_token[PIN_TOKEN_SIZE];

// PINトークンが生成済みかどうかを保持
static bool pin_token_generated = false;

// HMAC SHA-256ハッシュ格納領域
static uint8_t hmac[HMAC_SHA_256_SIZE];

void ctap2_client_pin_token_init(bool force)
{
    // PINトークンが生成済みで、かつ
    // 強制再生成を要求しない場合は終了
    if (pin_token_generated && force == false) {
        fido_log_debug("PIN token is already exist");
        return;
    }

    // 16バイトのランダムベクターを生成
    fido_crypto_generate_random_vector(m_pin_token, PIN_TOKEN_SIZE);

    // 生成済みフラグを設定
    if (!pin_token_generated) {
        fido_log_debug("PIN token generate success");
    } else {
        fido_log_debug("PIN token re-generate success");
    }
    pin_token_generated = true;
}

uint8_t *ctap2_client_pin_token_encoded(void)
{
    // PINトークンが未生成の場合は終了
    if (!pin_token_generated) {
        fido_log_debug("PIN token is not exist");
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

uint8_t ctap2_client_pin_token_verify_pin_auth(uint8_t *clientDataHash, uint8_t *pinAuth)
{
    // clientDataHashからHMAC SHA-256ハッシュを計算
    fido_crypto_calculate_hmac_sha256(m_pin_token, PIN_TOKEN_SIZE, 
        clientDataHash, CLIENT_DATA_HASH_SIZE, NULL, 0, hmac);

    // クライアントから受信したpinAuth（16バイト）を、
    // 生成されたHMAC SHA-256ハッシュと比較し、
    // 異なる場合はエラーを戻す
    if (memcmp(hmac, pinAuth, PIN_AUTH_SIZE) != 0) {
        return CTAP2_ERR_PIN_AUTH_INVALID;
    }

    return CTAP1_ERR_SUCCESS;
}
