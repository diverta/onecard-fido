/* 
 * File:   ctap2_common.c
 * Author: makmorit
 *
 * Created on 2019/01/03, 11:32
 */
#include "sdk_common.h"

#include "fido_crypto.h"
#include "ctap2_common.h"

// for u2f_crypto_sign & other
#include "u2f_crypto.h"

//
// CTAP2コマンドで共用する作業領域
// 
// RP IDのSHA-256ハッシュデータを保持
nrf_crypto_hash_sha256_digest_t ctap2_rpid_hash;
size_t                          ctap2_rpid_hash_size;

// flagsを保持
uint8_t ctap2_flags;

// signCountを保持
uint32_t ctap2_sign_count = 0;

// Public Key Credential Sourceを保持
uint8_t pubkey_cred_source[PUBKEY_CRED_SOURCE_MAX_SIZE];
size_t  pubkey_cred_source_block_size;

// credentialIdを保持
uint8_t credential_id[CREDENTIAL_ID_MAX_SIZE];
size_t  credential_id_size;

// credentialPublicKeyを保持
uint8_t credential_pubkey[CREDENTIAL_ID_MAX_SIZE];
size_t  credential_pubkey_size;

// Authenticator dataを保持
uint8_t authenticator_data[AUTHENTICATOR_DATA_MAX_SIZE];
size_t  authenticator_data_size;

void ctap2_generate_rpid_hash(uint8_t *rpid, size_t rpid_size)
{
    // RP IDからSHA-256ハッシュ（32バイト）を生成 
    ctap2_rpid_hash_size = sizeof(ctap2_rpid_hash);
    fido_crypto_generate_sha256_hash(rpid, rpid_size, ctap2_rpid_hash, &ctap2_rpid_hash_size);
}

bool ctap2_generate_signature(uint8_t *client_data_hash, uint8_t *private_key_be)
{
    // 例外抑止
    if (private_key_be == NULL) {
        return false;
    }

    // 署名生成用バッファの格納領域を取得
    uint8_t offset = 0;
    uint8_t *signature_base_buffer = u2f_crypto_signature_data_buffer();

    // Authenticator data
    memcpy(signature_base_buffer + offset, authenticator_data, authenticator_data_size);
    offset += authenticator_data_size;

    // clientDataHash 
    memcpy(signature_base_buffer + offset, client_data_hash, CLIENT_DATA_HASH_SIZE);
    offset += CLIENT_DATA_HASH_SIZE;

    // メッセージのバイト数をセット
    u2f_crypto_signature_data_size_set(offset);

    // 署名用の秘密鍵を使用し、署名を生成
    if (u2f_crypto_sign(private_key_be) != NRF_SUCCESS) {
        // 署名生成に失敗したら終了
        return false;
    }

    // ASN.1形式署名を格納する領域を準備
    if (u2f_crypto_create_asn1_signature() == false) {
        // 生成された署名をASN.1形式署名に変換する
        // 変換失敗の場合終了
        return false;
    }

    return true;
}
