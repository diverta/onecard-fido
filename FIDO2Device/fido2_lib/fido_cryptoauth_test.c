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
#include "fido_cryptoauth_setup.h"
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
#define FIDO_CRYPTOAUTH_TEST_FUNC  false
#define FIDO_CRYPTOAUTH_TEST_PRIVW true

#if FIDO_CRYPTOAUTH_TEST_FUNC
static uint8_t data[256];
static uint8_t hash_digest[ATCA_SHA_DIGEST_SIZE];
static size_t  hash_digest_size = ATCA_SHA_DIGEST_SIZE;
static uint8_t vector_buf[RANDOM_NUM_SIZE];
static uint8_t signature[ATCA_SIG_SIZE];
static size_t  signature_size = ATCA_SIG_SIZE;
static uint8_t *p_public_key;
static uint8_t hmac_digest[HMAC_DIGEST_SIZE];

static void generate_sha256_hash(void)
{
    // データを初期化
    memset(data, 0xbc, sizeof(data));

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

    // test end
    fido_cryptoauth_release();
}

static void sign_and_calculate_hmac_sha256(uint16_t priv_key_id)
{
    // データを初期化
    memset(data, 0xbc, sizeof(data));

    // 秘密鍵生成（移行コード）
    fido_cryptoauth_keypair_generate(priv_key_id);

    // HASHに署名（移行コード）
    fido_cryptoauth_ecdsa_sign(priv_key_id, hash_digest, signature, &signature_size);
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

    // test end
    fido_cryptoauth_release();
}

static void sskey_generate(void)
{
    // 秘密鍵生成
    fido_cryptoauth_sskey_init(false);
    fido_crypto_sskey_init(false);

    // ECDH共通鍵生成（既存コード）
    p_public_key = fido_cryptoauth_sskey_public_key();
    fido_crypto_sskey_generate(p_public_key);
    fido_log_debug("fido_crypto_sskey_generate:");
    fido_log_print_hexdump_debug(fido_crypto_sskey_hash(), 32);

    // ECDH共通鍵生成（移行コード）公開鍵はnRF cryptoで生成したものを使用
    p_public_key = fido_crypto_sskey_public_key();
    fido_cryptoauth_sskey_generate(p_public_key);
    fido_log_debug("fido_cryptoauth_sskey_generate:");
    fido_log_print_hexdump_debug(fido_cryptoauth_sskey_hash(), 32);

    // test end
    fido_cryptoauth_release();
}

static void test_functions(void)
{
    //
    // ボタンを押下するごとに機能を実行します。
    // （一息に実行すると、ログが途切れてしまうのを回避するための措置）
    //
    static uint8_t button_cnt = 0;
    switch (++button_cnt) {
        case 1:
            // SHA-256ハッシュ生成テスト
            generate_sha256_hash();
            break;
        case 2:
            // ECDSA署名 ＆ HMAC SHA-256ハッシュ生成テスト
            // 秘密鍵は 0 番スロットを使用
            sign_and_calculate_hmac_sha256(0);
            break;
        case 3:
            // ECDH共通鍵生成テスト
            sskey_generate();
            break;
        default:
            button_cnt = 0;
            fido_log_info("fido_cryptoauth_test_functions done");
            break;
    }
}
#endif // FIDO_CRYPTOAUTH_TEST_FUNC

#if FIDO_CRYPTOAUTH_TEST_PRIVW
static void test_privkey_write(void)
{
    ATCA_STATUS status = ATCA_SUCCESS;
    //uint8_t key_id       = 0x07;
    //uint8_t write_key_id = 0x04;
    uint8_t key_id       = 0x0e;
    uint8_t write_key_id = 0x0f;
    uint8_t public_key[64];

    static const uint8_t g_slot4_key[32] = {
        0x37, 0x80, 0xe6, 0x3d, 0x49, 0x68, 0xad, 0xe5, 0xd8, 0x22, 0xc0, 0x13, 0xfc, 0xc3, 0x23, 0x84,
        0x5d, 0x1b, 0x56, 0x9f, 0xe7, 0x05, 0xb6, 0x00, 0x06, 0xfe, 0xec, 0x14, 0x5a, 0x0d, 0xb1, 0xe3
    };
    static const uint8_t private_key[36] = {
        0x00, 0x00, 0x00, 0x00,
        0x87, 0x8F, 0x0A, 0xB6, 0xA5, 0x26, 0xD7, 0x11, 0x1C, 0x26, 0xE6, 0x17, 0x08, 0x10, 0x79, 0x6E,
        0x7B, 0x33, 0x00, 0x7F, 0x83, 0x2B, 0x8D, 0x64, 0x46, 0x7E, 0xD6, 0xF8, 0x70, 0x53, 0x7A, 0x19
    };
    static const uint8_t public_key_ref[64] = {
        0x8F, 0x8D, 0x18, 0x2B, 0xD8, 0x19, 0x04, 0x85, 0x82, 0xA9, 0x92, 0x7E, 0xA0, 0xC5, 0x6D, 0xEF,
        0xB4, 0x15, 0x95, 0x48, 0xE1, 0x1C, 0xA5, 0xF7, 0xAB, 0xAC, 0x45, 0xBB, 0xCE, 0x76, 0x81, 0x5B,
        0xE5, 0xC6, 0x4F, 0xCD, 0x2F, 0xD1, 0x26, 0x98, 0x54, 0x4D, 0xE0, 0x37, 0x95, 0x17, 0x26, 0x66,
        0x60, 0x73, 0x04, 0x61, 0x19, 0xAD, 0x5E, 0x11, 0xA9, 0x0A, 0xA4, 0x97, 0x73, 0xAE, 0xAC, 0x86
    };

    fido_cryptoauth_init();

    status = atcab_write_zone(ATCA_ZONE_DATA, write_key_id, 0, 0, g_slot4_key, 32);
    if (status != ATCA_SUCCESS) {
        fido_log_error("test_privkey_write failed: atcab_write_zone(%d) returns 0x%02x", 
            write_key_id, status);
    }
     
    status = atcab_priv_write(key_id, private_key, write_key_id, g_slot4_key);
    if (status != ATCA_SUCCESS) {
        fido_log_error("test_privkey_write failed: atcab_priv_write(%d) returns 0x%02x", 
            key_id, status);
    }

    status = atcab_get_pubkey(key_id, public_key);
    if (status != ATCA_SUCCESS) {
        fido_log_error("test_privkey_write failed: atcab_get_pubkey(%d) returns 0x%02x", 
            key_id, status);
    }

    int ret = memcmp(public_key_ref, public_key, sizeof(public_key_ref));
    fido_log_info("test_privkey_write: memcmp returns %d", ret);

    // test end
    fido_cryptoauth_release();
}
#endif // FIDO_CRYPTOAUTH_TEST_PRIVW

//
// for CRYPTOAUTH function test
//   テストコードのエントリーポイント
//   基板上のMAIN SWを押下すると実行されます。
//
void fido_cryptoauth_test_functions(void)
{
#if FIDO_CRYPTOAUTH_TEST_FUNC
    test_functions();
#endif // FIDO_CRYPTOAUTH_TEST_FUNC

#if FIDO_CRYPTOAUTH_TEST_PRIVW
    test_privkey_write();
#endif // FIDO_CRYPTOAUTH_TEST_PRIVW
}
