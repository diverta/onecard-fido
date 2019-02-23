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

// PINトークン格納領域
#define PIN_TOKEN_SIZE 16
static uint8_t m_pin_token[PIN_TOKEN_SIZE];

// PINトークンが生成済みかどうかを保持
static bool pin_token_generated = false;

void ctap2_client_pin_token_init(void)
{
    // 生成済みの場合は終了
    if (pin_token_generated) {
        NRF_LOG_DEBUG("PIN token is already exist");
        return;
    }

    // 16バイトのランダムベクターを生成
    memset(m_pin_token, 0, sizeof(m_pin_token));
    uint32_t err_code = nrf_drv_rng_rand(m_pin_token, PIN_TOKEN_SIZE);
    APP_ERROR_CHECK(err_code);

    // 生成済みフラグを設定
    NRF_LOG_DEBUG("PIN token generate success");
    pin_token_generated = true;
}

uint8_t *ctap2_client_pin_token_read(void)
{
    // PINトークン格納領域の先頭アドレスを戻す
    return m_pin_token;
}
