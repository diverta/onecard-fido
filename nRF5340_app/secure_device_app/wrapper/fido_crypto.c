/* 
 * File:   fido_crypto.c
 * Author: makmorit
 *
 * Created on 2021/09/16, 10:03
 */
//
// プラットフォーム非依存コード
//
#include "fido_common.h"
#include "fido_define.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// プラットフォーム依存コード
#include "app_crypto_define.h"
#include "app_crypto_ec.h"
#include "app_crypto_util.h"
#include "app_main.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(fido_crypto);
#endif

// for debug hex dump data
#define LOG_HEXDUMP_DEBUG_SSKEY     false
#define LOG_HEXDUMP_DEBUG_ENCRYPTED false
#define LOG_HEXDUMP_DEBUG_DECRYPTED false

//
// ハッシュ生成関連
//
void fido_crypto_generate_sha256_hash(uint8_t *data, size_t data_size, uint8_t *hash_digest, size_t *hash_digest_size)
{
    app_crypto_generate_sha256_hash(data, data_size, hash_digest);
    *hash_digest_size = SHA_256_HASH_SIZE;
}

void fido_crypto_calculate_hmac_sha256(uint8_t *key_data, size_t key_data_size, uint8_t *src_data, size_t src_data_size, uint8_t *src_data_2, size_t src_data_2_size, uint8_t *dest_data)
{
    app_crypto_generate_hmac_sha256(key_data, key_data_size, src_data, src_data_size, src_data_2, src_data_2_size, dest_data);
}

void fido_crypto_calculate_hmac_sha1(uint8_t *key_data, size_t key_data_size, uint8_t *src_data, size_t src_data_size, uint8_t *src_data_2, size_t src_data_2_size, uint8_t *dest_data)
{
    app_crypto_generate_hmac_sha1(key_data, key_data_size, src_data, src_data_size, src_data_2, src_data_2_size, dest_data);
}

void fido_crypto_random_pre_generate(void (*resume_func)(void))
{
    // ランダムベクターの生成を、専用スレッドで実行させるよう指示
    app_main_app_crypto_do_process(CRYPTO_EVT_RANDOM_PREGEN, resume_func);
}

void fido_crypto_generate_random_vector(uint8_t *vector_buf, size_t vector_buf_size)
{
    // Generate a random vector of specified length.
    app_crypto_generate_random_vector(vector_buf, vector_buf_size);
}

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
// ECDSA署名関連
//
bool fido_crypto_ecdsa_sign(uint8_t *private_key_be, uint8_t const *hash_digest, size_t digest_size, uint8_t *signature, size_t *signature_size)
{
    // 署名実行
    if (app_crypto_ec_dsa_sign(private_key_be, hash_digest, digest_size, signature) == false) {
        return false;
    }
    *signature_size = ECDSA_SIGNATURE_SIZE;
    return true;
}

bool fido_crypto_ecdsa_sign_verify(uint8_t *public_key_be, uint8_t const *hash_digest, size_t digest_size, uint8_t *signature, size_t signature_size)
{
    // 署名検証実行
    (void)signature_size;
    return app_crypto_ec_dsa_verify(public_key_be, hash_digest, digest_size, signature);
}

//
// 鍵交換関連
//
// 鍵交換用キーペア格納領域
//   この領域に格納される鍵は
//   ビッグエンディアン配列となる
static uint8_t private_key_raw_data_for_ss[RAW_PRIVATE_KEY_SIZE];
static uint8_t public_key_raw_data_for_ss[RAW_PUBLIC_KEY_SIZE];

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
static uint8_t sskey_raw_data[SHARED_SECRET_SIZE];
static uint8_t sskey_hash[SHA_256_HASH_SIZE];

uint8_t fido_crypto_sskey_generate(uint8_t *client_public_key_raw_data)
{
    // 鍵交換用キーペアが未生成の場合は終了
    if (!keypair_generated) {
        fido_log_error("Keypair for exchanging key is not exist");
        return CTAP1_ERR_OTHER;
    }

    // CTAP2クライアントから受け取った公開鍵と、自分で生成した秘密鍵を使用し、
    // 共通鍵を生成
    if (app_crypto_ec_calculate_ecdh(private_key_raw_data_for_ss, client_public_key_raw_data, sskey_raw_data, sizeof(sskey_raw_data)) == false) {
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

//
// AES-CBC暗号化関連
//
size_t fido_crypto_aes_cbc_256_decrypt(uint8_t *p_key, uint8_t *p_encrypted, size_t encrypted_size, uint8_t *decrypted) 
{
#if LOG_HEXDUMP_DEBUG_ENCRYPTED
    fido_log_debug("Encrypted data(%dbytes):", encrypted_size);
    fido_log_print_hexdump_debug(p_encrypted, encrypted_size, "");
#endif

    size_t decrypted_size;
    if (app_crypto_aes_cbc_256_decrypt(p_key, p_encrypted, encrypted_size, decrypted, &decrypted_size) == false) {
        return 0;
    }

#if LOG_HEXDUMP_DEBUG_DECRYPTED
    fido_log_debug("Decrypted data(%dbytes):", decrypted_size);
    fido_log_print_hexdump_debug(decrypted, decrypted_size, "");
#endif

    return decrypted_size;
}

size_t fido_crypto_aes_cbc_256_encrypt(uint8_t *p_key, uint8_t *p_plaintext, size_t plaintext_size, uint8_t *encrypted) 
{
    size_t encrypted_size;
    if (app_crypto_aes_cbc_256_encrypt(p_key, p_plaintext, plaintext_size, encrypted, &encrypted_size) == false) {
        return 0;
    }

    return encrypted_size;
}

bool fido_crypto_calculate_ecdh(uint8_t *private_key_raw_data, uint8_t *client_public_key_raw_data, uint8_t *sskey_raw_data, size_t *sskey_raw_data_size)
{
    return app_crypto_ec_calculate_ecdh(private_key_raw_data, client_public_key_raw_data, sskey_raw_data, *sskey_raw_data_size);
}

bool fido_crypto_tdes_enc(uint8_t *in, uint8_t *out, uint8_t *key)
{
    return app_crypto_des3_ecb(in, out, key);
}
