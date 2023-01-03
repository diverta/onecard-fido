/* 
 * File:   ctap2_common.c
 * Author: makmorit
 *
 * Created on 2019/01/03, 11:32
 */
#include <stddef.h>
#include <string.h>

//#include "ctap2_common.h"
#include "ctap2_define.h"
#include "fido_command_common.h"

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
static uint8_t authenticator_data[AUTHENTICATOR_DATA_MAX_SIZE];
static size_t  authenticator_data_size;

uint8_t *ctap2_authenticator_data(size_t *size)
{
    if (size != NULL) {
        *size = authenticator_data_size;
    }
    return authenticator_data;
}

void ctap2_authenticator_data_size_set(size_t size)
{
    authenticator_data_size = size;
}

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
    fido_command_calc_hash_sha256(rpid, rpid_size, ctap2_rpid_hash, &ctap2_rpid_hash_size);
}

void ctap2_generate_signature_base(uint8_t *client_data_hash)
{
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
