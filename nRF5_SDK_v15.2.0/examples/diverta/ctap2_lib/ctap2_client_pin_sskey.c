/* 
 * File:   ctap2_client_pin_sskey.c
 * Author: makmorit
 *
 * Created on 2019/02/23, 11:17
 */
#include "sdk_common.h"
#include "app_error.h"
#include "nrf_crypto_ecc.h"
#include "nrf_crypto_ecdh.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ctap2_client_pin_sskey
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for initial key pair generate
#include "fido_crypto_keypair.h"

// 鍵交換用キーペア格納領域
//   この領域に格納される鍵は
//   ビッグエンディアン配列となる
static uint8_t private_key_raw_data[NRF_CRYPTO_ECC_SECP256R1_RAW_PRIVATE_KEY_SIZE];
static uint8_t public_key_raw_data[NRF_CRYPTO_ECC_SECP256R1_RAW_PUBLIC_KEY_SIZE];

// 鍵交換用キーペアが生成済みかどうかを保持
static bool keypair_generated = false;

// SDK内部形式変換用の一時領域
//   CTAP2クライアントから受領した公開鍵
static nrf_crypto_ecc_public_key_t client_public_key;
//   鍵交換用キーペアの秘密鍵
static nrf_crypto_ecc_private_key_t self_private_key;

// 共通鍵格納領域
//   この領域に格納される共通鍵(Shared secret key)は、
//   ビッグエンディアン配列となる
static uint8_t sskey_raw_data[NRF_CRYPTO_ECDH_SECP256R1_SHARED_SECRET_SIZE];

void ctap2_client_pin_sskey_init(bool force)
{
    // 鍵交換用キーペアが生成済みで、かつ
    // 強制再生成を要求しない場合は終了
    if (keypair_generated && force == false) {
        NRF_LOG_DEBUG("Keypair for exchanging key is already exist");
        return;
    }

    // 秘密鍵および公開鍵を生成し、
    // モジュール変数内で保持
    fido_crypto_keypair_generate();
    memcpy(private_key_raw_data, fido_crypto_keypair_private_key(), sizeof(private_key_raw_data));
    memcpy(public_key_raw_data, fido_crypto_keypair_public_key, sizeof(public_key_raw_data));

    // 生成済みフラグを設定
    if (force) {
        NRF_LOG_DEBUG("Keypair for exchanging key re-generate success");
    } else {
        NRF_LOG_DEBUG("Keypair for exchanging key generate success");
    }
    keypair_generated = true;
}

void ctap2_client_pin_sskey_generate(uint8_t *client_public_key_raw_data)
{
    // 鍵交換用キーペアが未生成の場合は終了
    if (!keypair_generated) {
        NRF_LOG_DEBUG("PIN token is not exist");
        return;
    }

    // CTAP2クライアントから受け取った公開鍵を、SDK内部形式に変換
    ret_code_t err_code = nrf_crypto_ecc_public_key_from_raw(
        &g_nrf_crypto_ecc_secp256r1_curve_info, 
        &client_public_key, client_public_key_raw_data, 
        NRF_CRYPTO_ECC_SECP256R1_RAW_PUBLIC_KEY_SIZE);
    APP_ERROR_CHECK(err_code);

    // 自分で生成した公開鍵を、SDK内部形式に変換
    size_t size;
    err_code = nrf_crypto_ecc_private_key_from_raw(
        &g_nrf_crypto_ecc_secp256r1_curve_info, 
        &self_private_key, private_key_raw_data, 
        NRF_CRYPTO_ECC_SECP256R1_RAW_PRIVATE_KEY_SIZE);
    APP_ERROR_CHECK(err_code);

    // 共通鍵を生成
    size = NRF_CRYPTO_ECDH_SECP256R1_SHARED_SECRET_SIZE;
    err_code = nrf_crypto_ecdh_compute(NULL,
        &self_private_key, &client_public_key, sskey_raw_data, &size);
    APP_ERROR_CHECK(err_code);
}
