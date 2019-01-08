/* 
 * File:   ctap2_pubkey_credential.c
 * Author: makmorit
 *
 * Created on 2019/01/08, 11:24
 */
#include "sdk_common.h"

#include "ctap2_common.h"
#include "fido_common.h"
#include "fido_crypto.h"
#include "fido_crypto_ecb.h"
#include "fido_crypto_keypair.h"

// for u2f_flash_keydata_read & u2f_flash_keydata_available
#include "u2f_flash.h"

// for u2f_crypto_signature_data
#include "u2f_crypto.h"

// for u2f_securekey_skey_be
#include "u2f_register.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ctap2_pubkey_credential
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for debug cbor data
#define NRF_LOG_DEBUG_CRED_SOURCE   false
#define NRF_LOG_DEBUG_CREDENTIAL_ID false


// Public Key Credential Sourceを保持
static uint8_t pubkey_cred_source[PUBKEY_CRED_SOURCE_MAX_SIZE];
static size_t  pubkey_cred_source_block_size;

// credentialIdを保持
static uint8_t credential_id[CREDENTIAL_ID_MAX_SIZE];
static size_t  credential_id_size;

// Public Key Credential Sourceから
// 生成されたSHA-256ハッシュ値を保持
nrf_crypto_hash_sha256_digest_t credential_source_hash;
size_t                          credential_source_hash_size;

// RP IDに対応する
// CTAP_CREDENTIAL_DESC_T の個数を保持
static uint8_t number_of_credentials;

// credential IDから取り出した秘密鍵の
// 格納領域を保持
static uint8_t *private_key_be;

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
    // Public Key Credential Sourceから
    // SHA-256ハッシュ値（32バイト）を生成
    uint8_t pubkey_cred_source_size = pubkey_cred_source[0];
    credential_source_hash_size = sizeof(credential_source_hash);
    fido_crypto_generate_sha256_hash(
        pubkey_cred_source, pubkey_cred_source_size, credential_source_hash, &credential_source_hash_size);
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

void ctap2_pubkey_credential_generate_source(CTAP_PUBKEY_CRED_PARAM_T *param, CTAP_RP_ID_T *rp, CTAP_USER_ENTITY_T *user)
{
    // Public Key Credential Sourceを編集する
    // 
    //  0: Public Key Credential Source自体のサイズ
    //  1: Public Key Credential Type
    //  2 - 33: Credential private key（秘密鍵）
    //  34: Relying Party Identifierのサイズ
    //  35 - n: Relying Party Identifier（文字列）
    // 
    int offset = 1;
    memset(pubkey_cred_source, 0x00, sizeof(pubkey_cred_source));

    // Public Key Credential Type
    pubkey_cred_source[offset++] = param->publicKeyCredentialType;

    // Credential private key
    // キーペアを新規生成し、秘密鍵を格納
    fido_crypto_keypair_generate();
    memcpy(pubkey_cred_source + offset, 
        fido_crypto_keypair_private_key(), fido_crypto_keypair_private_key_size());
    offset += fido_crypto_keypair_private_key_size();

    // Relying Party Identifier (size & buffer)
    pubkey_cred_source[offset++] = rp->id_size;
    memcpy(pubkey_cred_source + offset, rp->id, rp->id_size);
    offset += rp->id_size;
    
    // Public Key Credential Source自体のサイズを、
    // バッファの１バイト目に設定
    pubkey_cred_source[0] = offset;

    // Public Key Credential Sourceから
    // SHA-256ハッシュ値（32バイト）を生成
    generate_credential_source_hash();

#if NRF_LOG_DEBUG_CRED_SOURCE
    NRF_LOG_DEBUG("Public Key Credential Source(%d bytes):", offset);
    NRF_LOG_HEXDUMP_DEBUG(pubkey_cred_source, offset);
#endif

    // 暗号化対象ブロックサイズを設定
    //   AES ECBの仕様上、16の倍数でなければならない
    int block_num = offset / 16;
    int block_sum = block_num * 16;
    if (offset == block_sum) {
        pubkey_cred_source_block_size = offset;
    } else {
        pubkey_cred_source_block_size = (block_num + 1) * 16;
    } 
}

void ctap2_pubkey_credential_generate_id(void)
{
    // Public Key Credential Sourceを
    // AES ECBで暗号化し、
    // credentialIdを生成する
    memset(credential_id, 0x00, sizeof(credential_id));
    fido_crypto_ecb_encrypt(pubkey_cred_source, pubkey_cred_source_block_size, credential_id);
    credential_id_size = pubkey_cred_source_block_size;

#if NRF_LOG_DEBUG_CREDENTIAL_ID
    NRF_LOG_DEBUG("credentialId(%d bytes):", credential_id_size);
    NRF_LOG_HEXDUMP_DEBUG(credential_id, credential_id_size);
#endif
}

static void ctap2_pubkey_credential_restore_source(uint8_t *credential_id, size_t credential_id_size)
{
    // authenticatorGetAssertionリクエストから取得した
    // credentialIdを復号化
    memset(pubkey_cred_source, 0, sizeof(pubkey_cred_source));
    fido_crypto_ecb_decrypt(credential_id, credential_id_size, pubkey_cred_source);

    // Public Key Credential Sourceから
    // SHA-256ハッシュ値（32バイト）を生成
    generate_credential_source_hash();

#if NRF_LOG_DEBUG_CRED_SOURCE
        NRF_LOG_DEBUG("Public Key Credential Source(%d bytes):", pubkey_cred_source[0]);
        NRF_LOG_HEXDUMP_DEBUG(pubkey_cred_source, pubkey_cred_source[0]);
#endif
}

static bool get_private_key_from_credential_id(CTAP_RP_ID_T *rp)
{
    // number_of_credentialsをゼロクリア
    number_of_credentials = 0;

    // Public Key Credential Sourceから
    // rpId(Relying Party Identifier)を取り出す。
    //  index
    //  2 - 33: Credential private key（秘密鍵）
    //  34: Relying Party Identifierのサイズ
    //  35 - n: Relying Party Identifier（文字列）
    // 
    size_t src_rp_id_size = pubkey_cred_source[34];
    char  *src_rp_id = (char *)(pubkey_cred_source + 35);

    // リクエストされたrpId
    size_t request_rp_id_size = rp->id_size;
    char  *request_rp_id = (char *)rp->id;

    // リクエストされたrpIdと、Public Key Credential Sourceの
    // rpIdを比較し、一致している場合
    src_rp_id_size = (src_rp_id_size < request_rp_id_size) ? src_rp_id_size : request_rp_id_size;
    if (strncmp(request_rp_id, src_rp_id, src_rp_id_size) == 0) {
        // number_of_credentialsをカウントアップ
        number_of_credentials++;
        // 秘密鍵をPublic Key Credential Sourceから取り出す
        private_key_be = pubkey_cred_source + 2;
#if NRF_LOG_DEBUG_CREDENTIAL_ID
        NRF_LOG_DEBUG("Private key of RP[%s]:", src_rp_id);
        NRF_LOG_HEXDUMP_DEBUG(private_key_be, 32);
#endif
        return true;

    } else {
        private_key_be = NULL;
        return false;
    }
}

uint8_t ctap2_pubkey_credential_restore_private_key(CTAP_ALLOW_LIST_T *allowList, CTAP_RP_ID_T *rp)
{
    int x;
    CTAP_CREDENTIAL_DESC_T *desc;

    // credentialIdリストの先頭から逐一処理
    for (x = 0; x < allowList->size; x++) {
        // credentialIdをAES ECBで復号化し、
        // Public Key Credential Sourceを取得
        desc = &(allowList->list[x]);
        ctap2_pubkey_credential_restore_source(desc->credential_id, desc->credential_id_size);

        // rpIdをマッチングさせ、一致していれば秘密鍵を取り出す
        if (get_private_key_from_credential_id(rp)) {
            pkey_credential_desc = desc;
            return CTAP1_ERR_SUCCESS;
        }
    }

    // credentialIdリストに
    // 一致するrpIdがない場合はエラー
    return CTAP2_ERR_PROCESSING;
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

CTAP_CREDENTIAL_DESC_T *ctap2_pubkey_credential_restored_id(void)
{
    // 秘密鍵の取出し元であるcredential IDの格納領域
    return pkey_credential_desc;
}
