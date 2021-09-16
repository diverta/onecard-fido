/* 
 * File:   app_fido.c
 * Author: makmorit
 *
 * Created on 2021/09/16, 10:03
 */

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(app_fido);
#endif

// Zephyrに依存する処理
#include "app_crypto_ec.h"

//
// EC鍵ペア関連
//
// 鍵ペア情報をRAWデータに変換する領域
//   この領域に格納される鍵は
//   ビッグエンディアン配列となる
static uint8_t private_key_raw_data[RAW_PRIVATE_KEY_SIZE];
static uint8_t public_key_raw_data[RAW_PUBLIC_KEY_SIZE];

void fido_crypto_keypair_generate(void)
{
    // キーペアを新規生成する
    app_crypto_ec_keypair_generate(private_key_raw_data, public_key_raw_data);
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
static uint8_t private_key_raw_data[RAW_PRIVATE_KEY_SIZE];
static uint8_t public_key_raw_data[RAW_PUBLIC_KEY_SIZE];

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
    fido_crypto_keypair_generate();
    memcpy(private_key_raw_data, fido_crypto_keypair_private_key(), sizeof(private_key_raw_data));
    memcpy(public_key_raw_data, fido_crypto_keypair_public_key(), sizeof(public_key_raw_data));

    // 生成済みフラグを設定
    if (!keypair_generated) {
        fido_log_debug("Keypair for exchanging key generate success");
    } else {
        fido_log_debug("Keypair for exchanging key re-generate success");
    }
    keypair_generated = true;
}
