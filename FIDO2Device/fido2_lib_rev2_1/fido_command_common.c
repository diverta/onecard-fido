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
#include "fido_common.h"

// for ATECC608A
#include "fido_cryptoauth.h"
#include "fido_cryptoauth_aes_cbc.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// CTAP2、U2Fで共用する各種処理
//
void fido_command_generate_random_vector(uint8_t *vector_buf, size_t vector_buf_size)
{
    fido_cryptoauth_generate_random_vector(vector_buf, vector_buf_size);
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
    bool exist;
    if (fido_cryptoauth_aes_cbc_check_password_exist(&exist) == false) {
        return false;
    }
    return exist;
}

size_t fido_command_aes_cbc_decrypt(uint8_t *p_encrypted, size_t encrypted_size, uint8_t *decrypted)
{
    size_t size = encrypted_size;
    if (fido_cryptoauth_aes_cbc_decrypt(p_encrypted, decrypted, &size) == false) {
        return 0;
    }
    return size;
}

size_t fido_command_aes_cbc_encrypt(uint8_t *p_plaintext, size_t plaintext_size, uint8_t *encrypted)
{
    size_t size = plaintext_size;
    if (fido_cryptoauth_aes_cbc_encrypt(p_plaintext, encrypted, &size) == false) {
        return 0;
    }
    return size;
}

//
// ハッシュ計算関連
//
void fido_command_calc_hash_sha256(uint8_t *data, size_t data_size, uint8_t *hash_digest, size_t *hash_digest_size)
{
    fido_cryptoauth_generate_sha256_hash(data, data_size, hash_digest, hash_digest_size);
}

void fido_command_calc_hash_hmac_sha256(
    uint8_t *key_data, size_t key_data_size, 
    uint8_t *src_data, size_t src_data_size, uint8_t *src_data_2, size_t src_data_2_size,
    uint8_t *dest_data)
{
    fido_cryptoauth_calculate_hmac_sha256(key_data, key_data_size, 
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
    // 未割り当てのスロットに、秘密鍵を新規生成
    return fido_cryptoauth_keypair_generate(u2f_keyhandle_new_privkey_id());
}

uint8_t *fido_command_keypair_pubkey_for_keyhandle(void)
{
    // fido_command_keypair_generate_for_keyhandle の
    // 実行により割り当てられたスロットの秘密鍵から、公開鍵を生成
    return fido_cryptoauth_keypair_public_key(u2f_keyhandle_get_privkey_id());
}

bool fido_command_keypair_generate_for_credential_id(void)
{
    // 未割り当てのスロットに、秘密鍵を新規生成
    return fido_cryptoauth_keypair_generate(ctap2_pubkey_credential_new_privkey_id());
}

uint8_t *fido_command_keypair_pubkey_for_credential_id(void)
{
    // fido_command_keypair_generate_for_credential_id の
    // 実行により割り当てられたスロットの秘密鍵から、公開鍵を生成
    return fido_cryptoauth_keypair_public_key(ctap2_pubkey_credential_get_privkey_id());
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
    fido_cryptoauth_sskey_init(force);
}

uint8_t *fido_command_sskey_public_key(void)
{
    return fido_cryptoauth_sskey_public_key();
}

uint8_t fido_command_sskey_generate(uint8_t *client_public_key_raw_data)
{
    if (fido_cryptoauth_sskey_generate(client_public_key_raw_data) == false) {
        return CTAP1_ERR_OTHER;
    }
    return CTAP1_ERR_SUCCESS;
}

size_t fido_command_sskey_aes_256_cbc_decrypt(uint8_t *p_encrypted, size_t encrypted_size, uint8_t *decrypted)
{
    return fido_crypto_aes_cbc_256_decrypt(fido_cryptoauth_sskey_hash(), p_encrypted, encrypted_size, decrypted);
}

size_t fido_command_sskey_aes_256_cbc_encrypt(uint8_t *p_plaintext, size_t plaintext_size, uint8_t *encrypted)
{
    return fido_crypto_aes_cbc_256_encrypt(fido_cryptoauth_sskey_hash(), p_plaintext, plaintext_size, encrypted);
}

void fido_command_sskey_calculate_hmac_sha256(
    uint8_t *src_data, size_t src_data_size, uint8_t *src_data_2, size_t src_data_2_size,
    uint8_t *dest_data)
{
    return fido_command_calc_hash_hmac_sha256(
        fido_cryptoauth_sskey_hash(), SSKEY_HASH_SIZE,
        src_data, src_data_size, src_data_2, src_data_2_size, dest_data);
}

//
// 署名関連
//
static uint8_t signature[ECDSA_SIGNATURE_SIZE];
static bool do_sign_with_privkey(uint16_t key_id)
{
    // 署名ベースからハッシュデータを生成
    u2f_signature_generate_hash_for_sign();

    // ハッシュデータと秘密鍵により、署名データ作成
    uint8_t *hash_digest = u2f_signature_hash_for_sign();
    size_t signature_size = ECDSA_SIGNATURE_SIZE;
    fido_cryptoauth_ecdsa_sign(key_id, hash_digest, signature, &signature_size);

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
    // 認証器固有の秘密鍵スロット番号を取得
    uint16_t private_key_id = KEY_ID_FOR_INSTALL_PRIVATE_KEY;

    // 署名ベースと秘密鍵により、署名データ作成
    return do_sign_with_privkey(private_key_id);
}

bool fido_command_do_sign_with_keyhandle(void)
{
    // キーハンドルから秘密鍵スロット番号を取り出す
    uint16_t private_key_id = u2f_keyhandle_privkey_id();

    // 署名ベースと秘密鍵により、署名データ作成
    return do_sign_with_privkey(private_key_id);
}

bool fido_command_do_sign_with_credential_id(void)
{
    // credential IDから秘密鍵スロット番号を取り出す
    uint16_t private_key_id = ctap2_pubkey_credential_privkey_id();

    // 署名ベースと秘密鍵により、署名データ作成
    return do_sign_with_privkey(private_key_id);
}
