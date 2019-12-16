/* 
 * File:   fido_cryptoauth_test.c
 * Author: makmorit
 *
 * Created on 2019/12/16, 10:20
 */
//
// プラットフォーム非依存コード
//
#include "fido_cryptoauth.h"
#include "fido_cryptoauth_test.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// ATECCx08A関連
//
#include "cryptoauthlib.h"

//
// for CRYPTOAUTH function test
//
#define FIDO_CRYPTOAUTH_TEST false

//
// for CRYPTOAUTH function test
//   CRYPTOAUTH 各種機能実行のためのテストコード
//
#if FIDO_CRYPTOAUTH_TEST
static uint8_t data[256];
static uint8_t hash_digest[ATCA_SHA_DIGEST_SIZE];
static size_t  hash_digest_size = ATCA_SHA_DIGEST_SIZE;
static uint8_t vector_buf[RANDOM_NUM_SIZE];
static uint8_t signature[ATCA_SIG_SIZE];
static size_t  signature_size = ATCA_SIG_SIZE;
static uint8_t *p_public_key;
static uint8_t hmac_digest[HMAC_DIGEST_SIZE];
#endif // FIDO_CRYPTOAUTH_TEST

void fido_cryptoauth_test_functions(void)
{
#if FIDO_CRYPTOAUTH_TEST
    static uint8_t button_cnt = 0;

    // ATECC608Aが接続されていない場合は終了
    // TODO:

    // データを初期化
    memset(data, 0xbc, sizeof(data));

    //
    // ボタンを押下するごとに機能を実行します。
    // （一息に実行すると、ログが途切れてしまうのを回避するための措置）
    //
    switch (++button_cnt) {
        case 1:
            // 32バイトのランダムベクターを作成（移行コード）
            fido_cryptoauth_generate_random_vector(vector_buf, sizeof(vector_buf));
            fido_log_debug("fido_cryptoauth_generate_random_vector (%d bytes):", sizeof(vector_buf));
            fido_log_print_hexdump_debug(vector_buf, sizeof(vector_buf));
            // HASH作成（既存コード）
            fido_crypto_generate_sha256_hash(data, sizeof(data), hash_digest, &hash_digest_size);
            fido_log_debug("fido_crypto_generate_sha256_hash (%d bytes):", hash_digest_size);
            fido_log_print_hexdump_debug(hash_digest, hash_digest_size);
            // HASH作成（移行コード）
            fido_cryptoauth_generate_sha256_hash(data, sizeof(data), hash_digest, &hash_digest_size);
            fido_log_debug("fido_cryptoauth_generate_sha256_hash (%d bytes):", hash_digest_size);
            fido_log_print_hexdump_debug(hash_digest, hash_digest_size);
            break;
        case 2:
            // 秘密鍵生成（移行コード）
            fido_cryptoauth_keypair_generate(0);
            // HASHに署名（移行コード）秘密鍵は 0 番を使用
            fido_cryptoauth_ecdsa_sign(0, hash_digest, signature, &signature_size);
            fido_log_debug("fido_cryptoauth_ecdsa_sign:");
            fido_log_print_hexdump_debug(signature, signature_size);
            // HMAC SHA-256ハッシュ生成（既存コード）
            fido_crypto_calculate_hmac_sha256(hash_digest, 16, 
                data, sizeof(data), NULL, 0, hmac_digest);
            fido_log_debug("fido_crypto_calculate_hmac_sha256 (%d bytes):", sizeof(hmac_digest));
            fido_log_print_hexdump_debug(hmac_digest, sizeof(hmac_digest));
            // HMAC SHA-256ハッシュ生成（移行コード）
            fido_cryptoauth_calculate_hmac_sha256(hash_digest, 16, 
                data, sizeof(data), NULL, 0, hmac_digest);
            fido_log_debug("fido_cryptoauth_calculate_hmac_sha256 (%d bytes):", sizeof(hmac_digest));
            fido_log_print_hexdump_debug(hmac_digest, sizeof(hmac_digest));
            break;
        case 3:
            // 秘密鍵生成
            fido_cryptoauth_sskey_init(false);
            fido_crypto_sskey_init(false);
            // ECDH共通鍵生成（既存コード）公開鍵は 1 番を使用
            p_public_key = fido_cryptoauth_sskey_public_key();
            fido_crypto_sskey_generate(p_public_key);
            fido_log_debug("fido_crypto_sskey_generate:");
            fido_log_print_hexdump_debug(fido_crypto_sskey_hash(), 32);
            // ECDH共通鍵生成（移行コード）公開鍵はnRF cryptoで生成したものを使用
            p_public_key = fido_crypto_sskey_public_key();
            fido_cryptoauth_sskey_generate(p_public_key);
            fido_log_debug("fido_cryptoauth_sskey_generate:");
            fido_log_print_hexdump_debug(fido_cryptoauth_sskey_hash(), 32);
            break;
        default:
            // test end
            fido_cryptoauth_release();
            button_cnt = 0;
            break;
    }
#endif // FIDO_CRYPTOAUTH_TEST
}
