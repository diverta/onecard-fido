/* 
 * File:   ctap2_common.c
 * Author: makmorit
 *
 * Created on 2019/01/03, 11:32
 */
#include <stddef.h>

#include "ctap2_common.h"

// for u2f_crypto_sign & other
#include "u2f_signature.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// CTAP2コマンドで共用する作業領域
// 
// RP IDのSHA-256ハッシュデータを保持
static uint8_t ctap2_rpid_hash[SHA_256_HASH_SIZE];
static size_t  ctap2_rpid_hash_size;

// flagsを保持
static uint8_t ctap2_flags;

// signCountを保持
static uint32_t ctap2_sign_count = 0;

// Authenticator dataを保持
uint8_t authenticator_data[AUTHENTICATOR_DATA_MAX_SIZE];
size_t  authenticator_data_size;

uint32_t ctap2_current_sign_count(void)
{
    return ctap2_sign_count;
}

void ctap2_set_sign_count(uint32_t count)
{
    ctap2_sign_count = count;
}

uint8_t *ctap2_generated_rpid_hash(void)
{
    return ctap2_rpid_hash;
}

size_t ctap2_generated_rpid_hash_size(void)
{
    return ctap2_rpid_hash_size;
}

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
    size_t  offset = 0;
    uint8_t *signature_base_buffer = u2f_signature_data_buffer();

    // Authenticator data
    memcpy(signature_base_buffer + offset, authenticator_data, authenticator_data_size);
    offset += authenticator_data_size;

    // clientDataHash 
    memcpy(signature_base_buffer + offset, client_data_hash, CLIENT_DATA_HASH_SIZE);
    offset += CLIENT_DATA_HASH_SIZE;

    // メッセージのバイト数をセット
    u2f_signature_base_data_size_set(offset);

    // 署名用の秘密鍵を使用し、署名を生成
    u2f_signature_do_sign(private_key_be);

    // ASN.1形式署名を格納する領域を準備
    if (u2f_signature_convert_to_asn1() == false) {
        // 生成された署名をASN.1形式署名に変換する
        // 変換失敗の場合終了
        return false;
    }

    return true;
}

uint8_t ctap2_flags_value(void)
{
    return ctap2_flags;
}

void ctap2_flags_init(uint8_t flag)
{
    ctap2_flags = flag;
}

void ctap2_flags_set(uint8_t flag)
{
    ctap2_flags |= flag;
}
