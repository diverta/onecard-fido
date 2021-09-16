/* 
 * File:   fido_command_common.c
 * Author: makmorit
 *
 * Created on 2020/02/06, 12:21
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// for ECDSA
#include "u2f_signature.h"

// for U2F keyhandle, CTAP2 credential id
#include "u2f.h"
#include "u2f_keyhandle.h"
#include "ctap2_pubkey_credential.h"

#ifdef CONFIG_USE_ATECC608A
// for ATECC608A
#include "atecc.h"
#include "atecc_aes.h"
#endif

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(fido_command_common);
#endif

//
// CTAP2、U2Fで共用する各種処理
//
void fido_command_generate_random_vector(uint8_t *vector_buf, size_t vector_buf_size)
{
    fido_crypto_generate_random_vector(vector_buf, vector_buf_size);
}

bool fido_command_check_skey_cert_exist(void)
{
    if (fido_flash_skey_cert_read() == false) {
        // 秘密鍵と証明書をFlash ROMから読込
        // NGであれば、エラーレスポンスを生成して戻す
        fido_log_error("Private key and certification read error");
        return false;
    }

    if (fido_flash_skey_cert_available() == false) {
        // 秘密鍵と証明書がFlash ROMに登録されていない場合
        // エラーレスポンスを生成して戻す
        return false;
    }
    
    // 秘密鍵と証明書が登録されている場合はtrue
    return true;
}

bool fido_command_check_aes_password_exist(void)
{
    if (fido_flash_password_get() == NULL) {
        // キーハンドルを暗号化するために必要な
        // AESパスワードが生成されていない場合
        // エラーレスポンスを生成して戻す
        return false;
    }

    // AESパスワードが生成されている場合はtrue
    return true;
}

size_t fido_command_aes_cbc_decrypt(uint8_t *p_encrypted, size_t encrypted_size, uint8_t *decrypted)
{
    // ATECC608Aが利用可能であれば、
    // ATECC608Aに登録されているAESパスワードを使用して復号化
    if (atecc_is_available()) {
        if (atecc_aes_decrypt(p_encrypted, encrypted_size, decrypted) == false) {
            return 0;
        } else {
            return encrypted_size;
        }
    }

    return fido_crypto_aes_cbc_256_decrypt(fido_flash_password_get(), p_encrypted, encrypted_size, decrypted);
}

size_t fido_command_aes_cbc_encrypt(uint8_t *p_plaintext, size_t plaintext_size, uint8_t *encrypted)
{
    // ATECC608Aが利用可能であれば、
    // ATECC608Aに登録されているAESパスワードを使用して暗号化
    if (atecc_is_available()) {
        if (atecc_aes_encrypt(p_plaintext, plaintext_size, encrypted) == false) {
            return 0;
        } else {
            return plaintext_size;
        }
    }

    return fido_crypto_aes_cbc_256_encrypt(fido_flash_password_get(), p_plaintext, plaintext_size, encrypted);
}

//
// ハッシュ計算関連
//
void fido_command_calc_hash_sha256(uint8_t *data, size_t data_size, uint8_t *hash_digest, size_t *hash_digest_size)
{
    fido_crypto_generate_sha256_hash(data, data_size, hash_digest, hash_digest_size);
}

void fido_command_calc_hash_hmac_sha256(
    uint8_t *key_data, size_t key_data_size, 
    uint8_t *src_data, size_t src_data_size, uint8_t *src_data_2, size_t src_data_2_size,
    uint8_t *dest_data)
{
    fido_crypto_calculate_hmac_sha256(key_data, key_data_size, 
        src_data, src_data_size, src_data_2, src_data_2_size, dest_data);
}

//
// 証明書関連
//
uint8_t *fido_command_cert_data(void)
{
    // 証明書データ格納領域の開始アドレスを取得
    return fido_flash_cert_data();
}

uint32_t fido_command_cert_data_length(void)
{
    // 証明書データ格納領域の長さを取得
    return fido_flash_cert_data_length();
}

//
// 公開鍵関連
//
bool fido_command_keypair_generate_for_keyhandle(void)
{
    fido_crypto_keypair_generate();
    return true;
}

uint8_t *fido_command_keypair_privkey_for_keyhandle(void)
{
    return fido_crypto_keypair_private_key();
}

uint8_t *fido_command_keypair_pubkey_for_keyhandle(void)
{
    return fido_crypto_keypair_public_key();
}

bool fido_command_keypair_generate_for_credential_id(void)
{
    fido_crypto_keypair_generate();
    return true;
}

uint8_t *fido_command_keypair_privkey_for_credential_id(void)
{
    return fido_crypto_keypair_private_key();
}

uint8_t *fido_command_keypair_pubkey_for_credential_id(void)
{
    return fido_crypto_keypair_public_key();
}

//
// 共通鍵関連
// ・共通鍵生成のためのキーペアを新規生成
// ・共通鍵を新規生成
// ・共通鍵によりデータを暗号化／復号化
// ・共通鍵を使用し、HMAC-SHA-256生成
//
void fido_command_sskey_init(bool force) 
{
    fido_crypto_sskey_init(force);
}

uint8_t *fido_command_sskey_public_key(void)
{
    return fido_crypto_sskey_public_key();
}

uint8_t fido_command_sskey_generate(uint8_t *client_public_key_raw_data)
{
    return fido_crypto_sskey_generate(client_public_key_raw_data);
}

size_t fido_command_sskey_aes_256_cbc_decrypt(uint8_t *p_encrypted, size_t encrypted_size, uint8_t *decrypted)
{
    return fido_crypto_aes_cbc_256_decrypt(fido_crypto_sskey_hash(), p_encrypted, encrypted_size, decrypted);
}

size_t fido_command_sskey_aes_256_cbc_encrypt(uint8_t *p_plaintext, size_t plaintext_size, uint8_t *encrypted)
{
    return fido_crypto_aes_cbc_256_encrypt(fido_crypto_sskey_hash(), p_plaintext, plaintext_size, encrypted);
}

void fido_command_sskey_calculate_hmac_sha256(
    uint8_t *src_data, size_t src_data_size, uint8_t *src_data_2, size_t src_data_2_size,
    uint8_t *dest_data)
{
    return fido_command_calc_hash_hmac_sha256(
        fido_crypto_sskey_hash(), SSKEY_HASH_SIZE,
        src_data, src_data_size, src_data_2, src_data_2_size, dest_data);
}

//
// 署名関連
//
static uint8_t signature[ECDSA_SIGNATURE_SIZE];

static bool do_sign_with_atecc_privkey(void)
{
    // 署名ベースからハッシュデータを生成
    u2f_signature_generate_hash_for_sign();

    // ハッシュデータと秘密鍵により、署名データ作成
    uint8_t *hash_digest = u2f_signature_hash_for_sign();
    if (atecc_generate_sign_with_privkey(KEY_ID_FOR_INSTALL_PRIVATE_KEY, hash_digest, signature) == false) {
        return false;
    }

    // ASN.1形式署名を格納する領域を準備
    if (u2f_signature_convert_to_asn1(signature) == false) {
        // 生成された署名をASN.1形式署名に変換する
        // 変換失敗の場合終了
        return false;
    }

    return true;
}

static bool do_sign_with_privkey(uint8_t *private_key_be)
{
    // 署名ベースからハッシュデータを生成
    u2f_signature_generate_hash_for_sign();

    // ハッシュデータと秘密鍵により、署名データ作成
    uint8_t *hash_digest = u2f_signature_hash_for_sign();
    size_t signature_size = ECDSA_SIGNATURE_SIZE;
    if (fido_crypto_ecdsa_sign(private_key_be, hash_digest, SHA_256_HASH_SIZE, signature, &signature_size) == false) {
        return false;
    }

    // ASN.1形式署名を格納する領域を準備
    if (u2f_signature_convert_to_asn1(signature) == false) {
        // 生成された署名をASN.1形式署名に変換する
        // 変換失敗の場合終了
        return false;
    }

    return true;
}

bool fido_command_do_sign_with_privkey(void)
{
    // ATECC608Aが利用可能であれば、
    // ATECC608Aに登録されている秘密鍵で署名データ作成
    if (atecc_is_available()) {
        return do_sign_with_atecc_privkey();
    }

    // 認証器固有の秘密鍵を取得
    uint8_t *private_key_be = fido_flash_skey_data();

    // 署名ベースと秘密鍵により、署名データ作成
    return do_sign_with_privkey(private_key_be);
}

bool fido_command_do_sign_with_keyhandle(void)
{
    // キーハンドルから秘密鍵を取り出す(33バイト目以降)
    uint8_t *private_key_be = u2f_keyhandle_base_buffer() + U2F_APPID_SIZE;

    // 署名ベースと秘密鍵により、署名データ作成
    return do_sign_with_privkey(private_key_be);
}

bool fido_command_do_sign_with_credential_id(void)
{
    // credential IDから秘密鍵を取り出す
    uint8_t *private_key_be = ctap2_pubkey_credential_private_key();

    // 署名ベースと秘密鍵により、署名データ作成
    return do_sign_with_privkey(private_key_be);
}

//
// 署名カウンター関連
//
bool fido_command_sign_counter_delete(void)
{
    return fido_flash_token_counter_delete();
}

bool fido_command_sign_counter_create(uint8_t *unique_key, uint8_t *rpid_hash, uint8_t *username)
{
    // カウンターを０として新規レコードを生成
    return fido_flash_token_counter_write(unique_key, 0, rpid_hash);
}

bool fido_command_sign_counter_read(uint8_t *unique_key)
{
    return fido_flash_token_counter_read(unique_key);
}

bool fido_command_sign_counter_update(uint8_t *unique_key, uint32_t counter)
{
    return fido_flash_token_counter_write(unique_key, counter, NULL);
}

uint32_t fido_command_sign_counter_value(void)
{
    return fido_flash_token_counter_value();
}

uint8_t *fido_command_sign_counter_get_rpid_hash(void)
{
    return fido_flash_token_counter_get_check_hash();
}

uint8_t *fido_command_sign_counter_get_username(void)
{
    return NULL;
}
