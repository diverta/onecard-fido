/* 
 * File:   app_fido.c
 * Author: makmorit
 *
 * Created on 2021/09/16, 10:03
 */
//
// プラットフォーム非依存コード
//
#include "fido_common.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(app_fido);
#endif

// for debug hex dump data
#define LOG_HEXDUMP_DEBUG_SSKEY     false

//
// EC鍵ペア関連
//
// 鍵ペア情報をRAWデータに変換する領域
//   この領域に格納される鍵は
//   ビッグエンディアン配列となる
static uint8_t private_key_raw_data[RAW_PRIVATE_KEY_SIZE];
static uint8_t public_key_raw_data[RAW_PUBLIC_KEY_SIZE+1];

static void remove_pubkey_header_byte(uint8_t *data)
{
    // 先頭バイト(0x04)を削除するため、１バイトずつ前にずらす
    for (uint8_t i = 0; i < RAW_PUBLIC_KEY_SIZE; i++) {
        data[i] = data[i + 1];
    }

    // 末尾に 0x00 を設定
    data[RAW_PUBLIC_KEY_SIZE] = 0x00;
}

void fido_crypto_keypair_generate(void)
{
    // キーペアを新規生成する
    app_crypto_ec_keypair_generate(private_key_raw_data, public_key_raw_data);
    remove_pubkey_header_byte(public_key_raw_data);
}

uint8_t *fido_crypto_keypair_private_key(void)
{
    return private_key_raw_data;
}

uint8_t *fido_crypto_keypair_public_key(void)
{
    return public_key_raw_data;
}

//
// 鍵交換関連
//
// 鍵交換用キーペア格納領域
//   この領域に格納される鍵は
//   ビッグエンディアン配列となる
static uint8_t private_key_raw_data_for_ss[RAW_PRIVATE_KEY_SIZE];
static uint8_t public_key_raw_data_for_ss[RAW_PUBLIC_KEY_SIZE+1];

uint8_t *fido_crypto_sskey_public_key(void)
{
    return public_key_raw_data_for_ss;
}

// 鍵交換用キーペアが生成済みかどうかを保持
static bool keypair_generated = false;

void fido_crypto_sskey_init(bool force)
{
    // 鍵交換用キーペアが生成済みで、かつ
    // 強制再生成を要求しない場合は終了
    if (keypair_generated && force == false) {
        fido_log_debug("Keypair for exchanging key is already exist");
        return;
    }

    // 秘密鍵および公開鍵を生成し、
    // モジュール変数内で保持
    app_crypto_ec_keypair_generate(private_key_raw_data_for_ss, public_key_raw_data_for_ss);
    remove_pubkey_header_byte(public_key_raw_data_for_ss);

    // 生成済みフラグを設定
    if (!keypair_generated) {
        fido_log_debug("Keypair for exchanging key generate success");
    } else {
        fido_log_debug("Keypair for exchanging key re-generate success");
    }
    keypair_generated = true;
}

//
// 共通鍵関連
//
// 共通鍵格納領域
//   この領域に格納される共通鍵(Shared secret key)は、
//   ビッグエンディアン配列となる
static uint8_t client_pubkey_work[RAW_PUBLIC_KEY_SIZE+1];
static uint8_t sskey_raw_data[SHARED_SECRET_SIZE];
static uint8_t sskey_hash[SHA_256_HASH_SIZE];

uint8_t fido_crypto_sskey_generate(uint8_t *client_public_key_raw_data)
{
    // 鍵交換用キーペアが未生成の場合は終了
    if (!keypair_generated) {
        fido_log_error("Keypair for exchanging key is not exist");
        return CTAP1_ERR_OTHER;
    }

    // 公開鍵の先頭に 0x04 を付加
    memcpy(client_pubkey_work + 1, client_public_key_raw_data, RAW_PUBLIC_KEY_SIZE);
    client_pubkey_work[0] = 0x04;
    
    // CTAP2クライアントから受け取った公開鍵と、自分で生成した秘密鍵を使用し、
    // 共通鍵を生成
    if (app_crypto_ec_calculate_ecdh(private_key_raw_data_for_ss, client_pubkey_work, sskey_raw_data, sizeof(sskey_raw_data)) == false) {
        return CTAP1_ERR_OTHER;
    }

#if LOG_HEXDUMP_DEBUG_SSKEY
    fido_log_debug("fido_crypto_sskey_generate:");
    fido_log_print_hexdump_debug(sskey_raw_data, sskey_raw_data_size, "");
#endif

    // 生成した共通鍵をSHA-256ハッシュ化し、
    // 共通鍵ハッシュ（32バイト）を作成
    if (app_crypto_generate_sha256_hash(sskey_raw_data, sizeof(sskey_raw_data), sskey_hash) == false) {
        return CTAP1_ERR_OTHER;
    }
    return CTAP1_ERR_SUCCESS;
}

uint8_t *fido_crypto_sskey_hash(void)
{
    return sskey_hash;
}
