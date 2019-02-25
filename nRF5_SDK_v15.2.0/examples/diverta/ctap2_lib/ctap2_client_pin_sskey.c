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
#include "nrf_crypto_hmac.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ctap2_client_pin_sskey
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for initial key pair generate
#include "fido_common.h"
#include "fido_crypto_keypair.h"
#include "fido_crypto.h"

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
static size_t  sskey_raw_data_size;

// 共通鍵ハッシュ格納領域
static nrf_crypto_hash_sha256_digest_t sskey_hash;
static size_t                          sskey_hash_size;

// HMAC SHA-256ハッシュ格納領域
static nrf_crypto_hmac_context_t hmac_context;
static uint8_t                   hmac_data[NRF_CRYPTO_HASH_SIZE_SHA256];
static size_t                    hmac_data_size;

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
    memcpy(public_key_raw_data, fido_crypto_keypair_public_key(), sizeof(public_key_raw_data));

    // 生成済みフラグを設定
    if (!keypair_generated) {
        NRF_LOG_DEBUG("Keypair for exchanging key generate success");
    } else {
        NRF_LOG_DEBUG("Keypair for exchanging key re-generate success");
    }
    keypair_generated = true;
}

uint8_t ctap2_client_pin_sskey_generate(uint8_t *client_public_key_raw_data)
{
    // 鍵交換用キーペアが未生成の場合は終了
    if (!keypair_generated) {
        NRF_LOG_ERROR("Keypair for exchanging key is not exist");
        return CTAP1_ERR_OTHER;
    }

    // CTAP2クライアントから受け取った公開鍵を、SDK内部形式に変換
    ret_code_t err_code = nrf_crypto_ecc_public_key_from_raw(
        &g_nrf_crypto_ecc_secp256r1_curve_info, 
        &client_public_key, client_public_key_raw_data, 
        NRF_CRYPTO_ECC_SECP256R1_RAW_PUBLIC_KEY_SIZE);
    APP_ERROR_CHECK(err_code);

    // 自分で生成した公開鍵を、SDK内部形式に変換
    err_code = nrf_crypto_ecc_private_key_from_raw(
        &g_nrf_crypto_ecc_secp256r1_curve_info, 
        &self_private_key, private_key_raw_data, 
        NRF_CRYPTO_ECC_SECP256R1_RAW_PRIVATE_KEY_SIZE);
    APP_ERROR_CHECK(err_code);

    // 共通鍵を生成
    sskey_raw_data_size = NRF_CRYPTO_ECDH_SECP256R1_SHARED_SECRET_SIZE;
    err_code = nrf_crypto_ecdh_compute(NULL,
        &self_private_key, &client_public_key, sskey_raw_data, &sskey_raw_data_size);
    APP_ERROR_CHECK(err_code);

    // 生成した共通鍵をSHA-256ハッシュ化し、
    // 共通鍵ハッシュ（32バイト）を作成
    sskey_hash_size = NRF_CRYPTO_HASH_SIZE_SHA256;
    fido_crypto_generate_sha256_hash(sskey_raw_data, sskey_raw_data_size, sskey_hash, &sskey_hash_size);
    return CTAP1_ERR_SUCCESS;
}

uint8_t *ctap2_client_pin_sskey_public_key(void)
{
    return public_key_raw_data;
}

uint8_t *ctap2_client_pin_sskey_hash(void)
{
    return sskey_hash;
}

uint8_t *ctap2_client_pin_sskey_calculate_hmac(
    uint8_t *src_data_1, size_t src_data_1_size,
    uint8_t *src_data_2, size_t src_data_2_size)
{
    // HMACハッシュ計算には、共通鍵ハッシュを使用
    ret_code_t err_code = nrf_crypto_hmac_init(
        &hmac_context, &g_nrf_crypto_hmac_sha256_info, sskey_hash, sskey_hash_size);
    APP_ERROR_CHECK(err_code);

    // 1番目の引数を計算対象に設定
    if (src_data_1 != NULL && src_data_1_size > 0) {
        err_code = nrf_crypto_hmac_update(&hmac_context, src_data_1, src_data_1_size);
        APP_ERROR_CHECK(err_code);
    }

    // 2番目の引数を計算対象に設定
    if (src_data_2 != NULL && src_data_2_size > 0) {
        err_code = nrf_crypto_hmac_update(&hmac_context, src_data_2, src_data_2_size);
        APP_ERROR_CHECK(err_code);
    }

    // HMACハッシュを計算
    hmac_data_size = sizeof(hmac_data);
    err_code = nrf_crypto_hmac_finalize(&hmac_context, hmac_data, &hmac_data_size);
    APP_ERROR_CHECK(err_code);

    // HMACハッシュの先頭アドレスを戻す
    return hmac_data;
}
