/* 
 * File:   fido_crypto_sskey.c
 * Author: makmorit
 *
 * Created on 2019/02/23, 11:17
 */
#include "sdk_common.h"
#include "app_error.h"
#include "nrf_crypto_ecc.h"
#include "nrf_crypto_ecdh.h"
#include "nrf_crypto_hash.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_crypto_sskey
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for debug hex dump data
#define LOG_HEXDUMP_DEBUG_SSKEY false

// for initial key pair generate
#include "fido_command_common.h"
#include "fido_common.h"
#include "fido_crypto.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

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

static nrf_crypto_ecdh_context_t nrf_crypto_ecdh_context;

static void app_error_check(char *function, ret_code_t err_code)
{
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("%s returns 0x%04x(%s)", 
            function, err_code, nrf_crypto_error_string_get(err_code));
        APP_ERROR_CHECK(err_code);
    }
}

void fido_crypto_sskey_init(bool force)
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

uint8_t fido_crypto_sskey_generate(uint8_t *client_public_key_raw_data)
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
    app_error_check("nrf_crypto_ecc_public_key_from_raw", err_code);

    // 自分で生成した公開鍵を、SDK内部形式に変換
    err_code = nrf_crypto_ecc_private_key_from_raw(
        &g_nrf_crypto_ecc_secp256r1_curve_info, 
        &self_private_key, private_key_raw_data, 
        NRF_CRYPTO_ECC_SECP256R1_RAW_PRIVATE_KEY_SIZE);
    app_error_check("nrf_crypto_ecc_private_key_from_raw", err_code);

    // 共通鍵を生成
    sskey_raw_data_size = NRF_CRYPTO_ECDH_SECP256R1_SHARED_SECRET_SIZE;
    err_code = nrf_crypto_ecdh_compute(&nrf_crypto_ecdh_context,
        &self_private_key, &client_public_key, sskey_raw_data, &sskey_raw_data_size);
    app_error_check("nrf_crypto_ecdh_compute", err_code);

#if LOG_HEXDUMP_DEBUG_SSKEY
    NRF_LOG_DEBUG("fido_crypto_sskey_generate:");
    NRF_LOG_HEXDUMP_DEBUG(sskey_raw_data, sskey_raw_data_size);
#endif

    // 生成した共通鍵をSHA-256ハッシュ化し、
    // 共通鍵ハッシュ（32バイト）を作成
    sskey_hash_size = NRF_CRYPTO_HASH_SIZE_SHA256;
    fido_command_calc_hash_sha256(sskey_raw_data, sskey_raw_data_size, sskey_hash, &sskey_hash_size);
    return CTAP1_ERR_SUCCESS;
}

uint8_t *fido_crypto_sskey_public_key(void)
{
    return public_key_raw_data;
}

uint8_t *fido_crypto_sskey_hash(void)
{
    return sskey_hash;
}
