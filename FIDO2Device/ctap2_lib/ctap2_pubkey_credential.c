/* 
 * File:   ctap2_pubkey_credential.c
 * Author: makmorit
 *
 * Created on 2019/01/08, 11:24
 */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ctap2_common.h"
#include "fido_command_common.h"
#include "fido_common.h"

// for u2f_crypto_signature_data
#include "u2f_signature.h"

// for u2f_securekey_skey_be
#include "u2f_register.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// for debug cbor data
#define LOG_DEBUG_CRED_SOURCE       false
#define LOG_DEBUG_CRED_SOURCE_BUFF  false
#define LOG_DEBUG_CREDENTIAL_ID     false
#define LOG_DEBUG_PRIVATE_KEY       false

// Public Key Credential Sourceを保持
static uint8_t pubkey_cred_source[PUBKEY_CRED_SOURCE_MAX_SIZE];
static size_t  pubkey_cred_source_block_size;

// credentialIdを保持
static uint8_t credential_id[CREDENTIAL_ID_MAX_SIZE];
static size_t  credential_id_size;

// Public Key Credential Sourceから
// 生成されたSHA-256ハッシュ値を保持
uint8_t credential_source_hash[SHA_256_HASH_SIZE];
size_t  credential_source_hash_size;

// RP IDに対応する
// CTAP_CREDENTIAL_DESC_T の個数を保持
static uint8_t number_of_credentials;

// credential IDから取り出した秘密鍵の
// 格納領域を保持
static uint8_t *private_key_be;

// credential IDから取り出したCredRandomの
// 格納領域を保持
static uint8_t *cred_random;

// 秘密鍵の取出し元であるcredential IDの
// 格納領域を保持
static CTAP_CREDENTIAL_DESC_T *pkey_credential_desc;


uint8_t *ctap2_pubkey_credential_id(void)
{
    return credential_id;
}

size_t ctap2_pubkey_credential_id_size(void)
{
    return credential_id_size;
}

static void generate_credential_source_hash()
{
    // rpIdHashの先頭アドレスとサイズを取得
    uint8_t *ctap2_rpid_hash = ctap2_generated_rpid_hash();
    size_t   ctap2_rpid_hash_size = ctap2_generated_rpid_hash_size();

    // Public Key Credential Sourceの後ろに
    // rpIdHashを連結
    size_t   pubkey_cred_source_size = pubkey_cred_source[0];
    uint8_t *hash_source_buffer = pubkey_cred_source;
    memcpy(hash_source_buffer + pubkey_cred_source_size, ctap2_rpid_hash, ctap2_rpid_hash_size);
    
    // Public Key Credential Source + rpIdHashから
    // SHA-256ハッシュ値（32バイト）を生成
    size_t hash_source_size = pubkey_cred_source_size + ctap2_rpid_hash_size;
    credential_source_hash_size = sizeof(credential_source_hash);
    fido_command_calc_hash_sha256(
        hash_source_buffer, hash_source_size, credential_source_hash, &credential_source_hash_size);
}

uint8_t *ctap2_pubkey_credential_source_hash(void)
{
    // Public Key Credential Sourceから
    // 生成されたSHA-256ハッシュ値を戻す
    //   トークンカウンター登録／更新用の
    //   キーとして利用する目的
    return credential_source_hash;
}

size_t ctap2_pubkey_credential_source_hash_size(void)
{
    return credential_source_hash_size;
}

void ctap2_pubkey_credential_generate_source(CTAP_PUBKEY_CRED_PARAM_T *param, CTAP_USER_ENTITY_T *user)
{
    // Public Key Credential Sourceを編集する
    // 
    //  0: Public Key Credential Source自体のサイズ
    //  1: Public Key Credential Type
    //  2 - 33: Credential private key（秘密鍵）
    //  34: User Id（バイト配列）のサイズ
    //  35 - n: User Id（バイト配列）
    //  n+1 - n+32: CredRandom（32バイト）
    // 
    size_t offset = 1;
    memset(pubkey_cred_source, 0x00, sizeof(pubkey_cred_source));

    // Public Key Credential Type
    pubkey_cred_source[offset++] = param->publicKeyCredentialType;

    // Credential private key
    // 新規生成したキーペアの秘密鍵を格納
    memcpy(pubkey_cred_source + offset, 
        fido_command_keypair_privkey_for_credential_id(), CTAP2_PRIVKEY_SIZE);
    offset += CTAP2_PRIVKEY_SIZE;

    // User Id (size & buffer)
    pubkey_cred_source[offset++] = user->id_size;
    memcpy(pubkey_cred_source + offset, user->id, user->id_size);
    offset += user->id_size;

    // CredRandom（32バイトのランダムなバイト配列）をセット
    fido_command_generate_random_vector(pubkey_cred_source + offset, CRED_RANDOM_SIZE);
    offset += CRED_RANDOM_SIZE;

    // BLE近接認証機能用のスキャンパラメーターを末尾に追加
    //  先頭バイト: パラメーター長
    //  後続バイト: パラメーターのバイト配列を格納
    offset += ble_peripheral_auth_scan_param_prepare(pubkey_cred_source + offset);

#if LOG_DEBUG_CRED_SOURCE
    fido_log_debug("Public Key Credential Source contents");
    fido_log_debug("USER ID (%d bytes):", user->id_size);
    fido_log_print_hexdump_debug(user->id, user->id_size);
#endif

    // Public Key Credential Source自体のサイズを、
    // バッファの１バイト目に設定
    pubkey_cred_source[0] = offset;

    // Public Key Credential Sourceから
    // SHA-256ハッシュ値（32バイト）を生成
    generate_credential_source_hash();

#if LOG_DEBUG_CRED_SOURCE_BUFF
    fido_log_debug("Public Key Credential Source(%d bytes):", offset);
    print_hexdump_debug(pubkey_cred_source, offset);
#endif

    // 暗号化対象ブロックサイズを設定
    //   AESの仕様上、16の倍数でなければならない
    pubkey_cred_source_block_size = fido_calculate_aes_block_size(offset);
}

void ctap2_pubkey_credential_generate_id(void)
{
    // Public Key Credential Sourceを
    // AES CBCで暗号化し、
    // credentialIdを生成する
    memset(credential_id, 0x00, sizeof(credential_id));
    fido_command_aes_cbc_encrypt(pubkey_cred_source, pubkey_cred_source_block_size, credential_id);
    credential_id_size = pubkey_cred_source_block_size;

#if LOG_DEBUG_CREDENTIAL_ID
    fido_log_debug("credentialId(%d bytes):", credential_id_size);
    fido_log_print_hexdump_debug(credential_id, credential_id_size);
#endif
}

static void ctap2_pubkey_credential_restore_source(uint8_t *credential_id, size_t credential_id_size)
{
    // authenticatorGetAssertionリクエストから取得した
    // credentialIdを復号化
    memset(pubkey_cred_source, 0, sizeof(pubkey_cred_source));
    fido_command_aes_cbc_decrypt(credential_id, credential_id_size, pubkey_cred_source);

    // Public Key Credential Sourceから
    // SHA-256ハッシュ値（32バイト）を生成
    generate_credential_source_hash();

#if LOG_DEBUG_CRED_SOURCE_BUFF
    fido_log_debug("Public Key Credential Source(%d bytes):", pubkey_cred_source[0]);
    print_hexdump_debug(pubkey_cred_source, pubkey_cred_source[0]);
#endif
}

static bool get_private_key_from_credential_id(void)
{
    // number_of_credentialsをゼロクリア
    number_of_credentials = 0;

    // Public Key Credential Sourceから
    // rpId(Relying Party Identifier)を取り出す。
    //  index
    //  0: Public Key Credential Source自体のサイズ
    //  1: Public Key Credential Type
    //  2 - 33: Credential private key（秘密鍵）
    //  34: User Id（バイト配列）のサイズ
    //  35 - n: User Id（バイト配列）
    //  n+1 - n+32: CredRandom（32バイト）
    // 
#if LOG_DEBUG_CRED_SOURCE
    size_t offset = 34;
    size_t src_user_id_size = pubkey_cred_source[offset++];
    char  *src_user_id = (char *)(pubkey_cred_source + offset);

    fido_log_debug("Public Key Credential Source contents");
    fido_log_debug("USER ID (%d bytes):", src_user_id_size);
    fido_log_print_hexdump_debug(src_user_id, src_user_id_size);
#endif

    // number_of_credentialsをカウントアップ
    number_of_credentials++;
    // 秘密鍵をPublic Key Credential Sourceから取り出す
    private_key_be = pubkey_cred_source + 2;
#if LOG_DEBUG_PRIVATE_KEY
    fido_log_debug("Private key:", src_rp_id);
    fido_log_print_hexdump_debug(private_key_be, 32);
#endif

    // CredRandom領域を取り出す
    // （ユーザーID末尾の次から32バイト分）
    uint8_t index = 34;
    uint8_t userid_size = pubkey_cred_source[index];
    index += (1 + userid_size);
    cred_random = pubkey_cred_source + index;

    return true;
}

uint8_t ctap2_pubkey_credential_restore_private_key(CTAP_ALLOW_LIST_T *allowList)
{
    int x;
    CTAP_CREDENTIAL_DESC_T *desc;

    // credentialIdリストの先頭から逐一処理
    for (x = 0; x < allowList->size; x++) {
        // credentialIdをAES CBCで復号化し、
        // Public Key Credential Sourceを取得
        //   Public Key Credential Source + rpIdHash からの
        //   ハッシュ生成も同時に実行
        desc = &(allowList->list[x]);
        ctap2_pubkey_credential_restore_source(desc->credential_id, desc->credential_id_size);

        // Public Key Credential Source + rpIdHash から
        // 生成されたSHA-256ハッシュ値をキーとし、
        // 署名カウンター情報を検索
        uint8_t *p_hash = ctap2_pubkey_credential_source_hash();
        if (fido_command_sign_counter_read(p_hash) == false) {
            // 紐づく署名カウンター情報がない場合は
            // 次のリスト要素をチェック
            continue;
        }

        // 秘密鍵を取り出す
        if (get_private_key_from_credential_id()) {
            pkey_credential_desc = desc;
            return CTAP1_ERR_SUCCESS;
        }
    }

    // credentialIdリストに
    // 一致するrpIdがない場合はエラー
    return CTAP2_ERR_NO_CREDENTIALS;
}

uint8_t ctap2_pubkey_credential_number(void)
{
    // RP IDに対応する
    // CTAP_CREDENTIAL_DESC_T の個数
    return number_of_credentials;
}

uint8_t *ctap2_pubkey_credential_private_key(void)
{
    // credential IDから取り出した秘密鍵の格納領域
    return private_key_be;
}

uint8_t *ctap2_pubkey_credential_cred_random(void)
{
    // credential IDから取り出したCredRandomの格納領域
    return cred_random;
}

CTAP_CREDENTIAL_DESC_T *ctap2_pubkey_credential_restored_id(void)
{
    // 秘密鍵の取出し元であるcredential IDの格納領域
    return pkey_credential_desc;
}

uint8_t *ctap2_pubkey_credential_ble_auth_scan_param(void)
{
    // Public Key Credential Source における
    // User Id（バイト配列）のサイズを取得
    size_t offset = 34;
    size_t src_user_id_size = pubkey_cred_source[offset];

    // BLEスキャンパラメーター格納領域の開始インデックスを取得
    offset = offset + 1 + src_user_id_size + 32;

    // BLEスキャンパラメーター格納領域の先頭を戻す
    return (pubkey_cred_source + offset);
}
