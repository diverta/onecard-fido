/* 
 * File:   ctap2_client_pin_sskey.c
 * Author: makmorit
 *
 * Created on 2019/02/23, 11:17
 */
#include "sdk_common.h"
#include <stdio.h>
#include <string.h>

#include "fido_crypto_keypair.h"

// for keysize informations
#include "nrf_crypto_ecc.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ctap2_client_pin_sskey
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// 鍵交換用キーペア格納領域
//   この領域に格納される鍵は
//   ビッグエンディアン配列となる
static uint8_t private_key_raw_data[NRF_CRYPTO_ECC_SECP256R1_RAW_PRIVATE_KEY_SIZE];
static uint8_t public_key_raw_data[NRF_CRYPTO_ECC_SECP256R1_RAW_PUBLIC_KEY_SIZE];

// 鍵交換用キーペアが生成済みかどうかを保持
static bool keypair_generated = false;

void ctap2_client_pin_sskey_init(void)
{
    // 生成済みの場合は終了
    if (keypair_generated) {
        NRF_LOG_DEBUG("Keypair for exchanging key is already exist");
        return;
    }

    // 秘密鍵および公開鍵を生成し、
    // モジュール変数内で保持
    fido_crypto_keypair_generate();
    memcpy(private_key_raw_data, fido_crypto_keypair_private_key(), sizeof(private_key_raw_data));
    memcpy(public_key_raw_data, fido_crypto_keypair_public_key, sizeof(public_key_raw_data));
    
    // 生成済みフラグを設定
    NRF_LOG_DEBUG("Keypair for exchanging key generate success");
    keypair_generated = true;
}