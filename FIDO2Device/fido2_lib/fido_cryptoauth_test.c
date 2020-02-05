/* 
 * File:   fido_cryptoauth_test.c
 * Author: makmorit
 *
 * Created on 2019/12/16, 10:20
 */
#if ATECC608A_ENABLED
//
// プラットフォーム非依存コード
//
#include "fido_cryptoauth.h"
#include "fido_cryptoauth_aes_cbc.h"
#include "fido_cryptoauth_setup.h"
#include "fido_cryptoauth_test.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// ATECCx08A関連
//
#include "cryptoauthlib.h"

#define CREATE_NEW_PASSWORD false

//
// for CRYPTOAUTH function test
//
static uint8_t data[256];
static uint8_t hash_digest[ATCA_SHA_DIGEST_SIZE];
static size_t  hash_digest_size = ATCA_SHA_DIGEST_SIZE;
static uint8_t vector_buf[RANDOM_NUM_SIZE];
static uint8_t signature[ATCA_SIG_SIZE];
static size_t  signature_size = ATCA_SIG_SIZE;
static uint8_t *p_public_key;
static uint8_t hmac_digest[HMAC_DIGEST_SIZE];

// for AES-128-CBC test
static uint8_t plaintext[64];
static uint8_t ciphertext[64];
static uint8_t decrypttext[64];

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

static void test_privkey_write(void)
{
    uint8_t public_key_ref[ATCA_PUB_KEY_SIZE];

    // Flash ROMに登録済みのデータがあれば領域に読込
    if (fido_flash_skey_cert_read() == false) {
        return;
    }

    // Flash ROMに導入されている外部証明書を取得し、公開鍵を抽出
    if (fido_cryptoauth_extract_pubkey_from_cert(public_key_ref, fido_flash_cert_data(), fido_flash_cert_data_length()) == false) {
        return;
    }

    // Flash ROMに導入されている外部秘密鍵を取得し、
    // １４番スロットにインストール
    uint8_t *private_key = fido_flash_skey_data();
    if (fido_cryptoauth_install_privkey(private_key) == false) {
        return;
    }

    // Flash ROMにインストール済みの外部秘密鍵と公開鍵をダンプ
    fido_log_debug("Private key of certificate (installed into ATECC608A):");
    fido_log_print_hexdump_debug(private_key, ATCA_PRIV_KEY_SIZE);

    fido_log_debug("Public key of certificate:");
    fido_log_print_hexdump_debug(public_key_ref, ATCA_PUB_KEY_SIZE);

    // １４番スロットの秘密鍵から、公開鍵を生成
    uint8_t *p_public_key = fido_cryptoauth_keypair_public_key(KEY_ID_FOR_INSTALL_PRIVATE_KEY);

    // 生成された公開鍵をダンプ
    fido_log_debug("Public key from ATECC608A:");
    fido_log_print_hexdump_debug(p_public_key, ATCA_PUB_KEY_SIZE);

    // 内容を検証
    if (memcmp(public_key_ref, p_public_key, sizeof(public_key_ref)) != 0) {
        fido_log_error("test_privkey_write failed: Invalid public key");
    }

    // test end
    fido_cryptoauth_release();
}

static void test_aes_cbc(void)
{
#if CREATE_NEW_PASSWORD
    // AESパスワード新規生成
    if (fido_cryptoauth_aes_cbc_new_password() == false) {
        return;
    }
#endif

    // 64バイトのランダムベクターを作成
    fido_cryptoauth_generate_random_vector(plaintext, 32);
    fido_cryptoauth_generate_random_vector(plaintext + 32, 32);
    fido_log_debug("Plain text:");
    fido_log_print_hexdump_debug(plaintext, sizeof(plaintext));

    // AESパスワードで暗号化
    size_t size = sizeof(plaintext);
    if (fido_cryptoauth_aes_cbc_encrypt(plaintext, ciphertext, &size) == false) {
        return;
    }
    fido_log_debug("Encrypted text:");
    fido_log_print_hexdump_debug(ciphertext, sizeof(ciphertext));

    // AESパスワードで復号化
    size = sizeof(decrypttext);
    if (fido_cryptoauth_aes_cbc_decrypt(ciphertext, decrypttext, &size) == false) {
        return;
    }
    fido_log_debug("Decrypted text:");
    fido_log_print_hexdump_debug(decrypttext, sizeof(decrypttext));

    // test end
    fido_cryptoauth_release();
}

//
// for CRYPTOAUTH function test
//   テストコードのエントリーポイント
//   基板上のMAIN SWを押下すると実行されます。
//
void fido_cryptoauth_test_functions(void)
{
    //
    // ボタンを押下するごとに機能を実行します。
    // （一息に実行すると、ログが途切れてしまうのを回避するための措置）
    //
    static uint8_t button_cnt = 0;
    switch (++button_cnt) {
        case 1:
            // 外部秘密鍵導入テスト
            test_privkey_write();
            break;
        case 2:
            // SHA-256ハッシュ生成テスト
            generate_sha256_hash();
            break;
        case 3:
            // ECDSA署名 ＆ HMAC SHA-256ハッシュ生成テスト
            // 秘密鍵は 0 番スロットを使用
            sign_and_calculate_hmac_sha256(0);
            break;
        case 4:
            // ECDH共通鍵生成テスト
            sskey_generate();
            break;
        case 5:
            // AESパスワードによる暗号化／復号化テスト
            test_aes_cbc();
            break;
        default:
            button_cnt = 0;
            fido_log_info("fido_cryptoauth_test_functions done");
            break;
    }
}

#endif // ATECC608A_ENABLED
