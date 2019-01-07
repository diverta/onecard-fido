/* 
 * File:   ctap2_common.c
 * Author: makmorit
 *
 * Created on 2019/01/03, 11:32
 */
#include "sdk_common.h"

#include "fido_crypto.h"
#include "ctap2_common.h"

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
