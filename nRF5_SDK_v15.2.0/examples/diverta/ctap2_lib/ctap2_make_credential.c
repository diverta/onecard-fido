/* 
 * File:   ctap2_make_credential.c
 * Author: makmorit
 *
 * Created on 2018/12/25, 11:33
 */
#include "sdk_common.h"

#include <stdbool.h>
#include "cbor.h"
#include "ctap2.h"
#include "ctap2_cbor_parse.h"
#include "fido_common.h"
#include "fido_crypto.h"
#include "fido_crypto_ecb.h"
#include "fido_crypto_keypair.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ctap2_make_credential
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for debug cbor data
#define NRF_LOG_HEXDUMP_DEBUG_CBOR false
#define NRF_LOG_DEBUG_CBOR_CONTENT true

// デコードされた
// authenticatorMakeCredential
// リクエストデータを保持する構造体
struct {
    uint32_t                 paramsParsed;
    uint8_t                  clientDataHash[CLIENT_DATA_HASH_SIZE];
    CTAP_RP_ID_T             rp;
    CTAP_USER_ENTITY_T       user;
    CTAP_PUBKEY_CRED_PARAM_T cred_param;
    CTAP_OPTIONS_T           options;
} make_credential_request;

// Public Key Credential Sourceを保持
static uint8_t pubkey_cred_source[128];
static size_t  pubkey_cred_source_block_size;

// credentialIdを保持
static uint8_t credential_id[128];
static size_t  credential_id_size;

// エンコードされたかどうかを保持するビット
#define PARAM_clientDataHash    (1 << 0)
#define PARAM_rp                (1 << 1)
#define PARAM_user              (1 << 2)
#define PARAM_pubKeyCredParams  (1 << 3)
#define PARAM_excludeList       (1 << 4)
#define PARAM_extensions        (1 << 5)
#define PARAM_options           (1 << 6)
#define PARAM_pinAuth           (1 << 7)
#define PARAM_pinProtocol       (1 << 8)
#define PARAM_rpId              (1 << 9)
#define PARAM_allowList         (1 << 10)

uint8_t ctap2_make_credential_decode_request(uint8_t *cbor_data_buffer, size_t cbor_data_length)
{
    CborParser  parser;
    CborValue   it;
    CborValue   map;
    size_t      map_length;
    CborType    type;
    int         ret;
    int         i;
    int         key;

#if NRF_LOG_HEXDUMP_DEBUG_CBOR
    NRF_LOG_HEXDUMP_DEBUG(cbor_data_buffer, 64);
    NRF_LOG_HEXDUMP_DEBUG(cbor_data_buffer + 64, cbor_data_length - 64);
#endif

    // リクエスト格納領域初期化
    memset(&make_credential_request, 0x00, sizeof(make_credential_request));
    
    // CBOR parser初期化
    ret = cbor_parser_init(cbor_data_buffer, cbor_data_length, CborValidateCanonicalFormat, &parser, &it);
    if (ret != CborNoError) {
        return CTAP2_ERR_CBOR_PARSING;
    }

    type = cbor_value_get_type(&it);
    if (type != CborMapType) {
        return CTAP2_ERR_CBOR_UNEXPECTED_TYPE;
    }

    ret = cbor_value_enter_container(&it, &map);
    if (ret != CborNoError) {
        return CTAP2_ERR_CBOR_PARSING;
    }

    ret = cbor_value_get_map_length(&it, &map_length);
    if (ret != CborNoError) {
        return CTAP2_ERR_CBOR_PARSING;
    }

    for (i = 0; i < map_length; i++) {
        type = cbor_value_get_type(&map);
        if (type != CborIntegerType) {
            return CTAP2_ERR_CBOR_UNEXPECTED_TYPE;
        }
        ret = cbor_value_get_int_checked(&map, &key);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }
        ret = cbor_value_advance(&map);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }

        ret = 0;
        switch(key) {
            case 1:
                // clientDataHash (Byte Array)
                ret = parse_fixed_byte_string(&map, make_credential_request.clientDataHash, CLIENT_DATA_HASH_SIZE);
                if (ret == CborNoError) {
                    make_credential_request.paramsParsed |= PARAM_clientDataHash;
                }
                break;
            case 2:
                // rp (PublicKeyCredentialRpEntity)
                ret = parse_rp(&make_credential_request.rp, &map);
                if (ret == CborNoError) {
                    make_credential_request.paramsParsed |= PARAM_rp;
                }
                break;
            case 3:
                // user (PublicKeyCredentialUserEntity)
                ret = parse_user(&make_credential_request.user, &map);
                if (ret == CborNoError) {
                    make_credential_request.paramsParsed |= PARAM_user;
                }
                break;
            case 4:
                // pubKeyCredParams (CBOR Array)
                ret = parse_pub_key_cred_params(&make_credential_request.cred_param, &map);
                if (ret == CborNoError) {
                    make_credential_request.paramsParsed |= PARAM_pubKeyCredParams;
                }
                break;                
            case 7:
                // options (Map of authenticator options)
                ret = parse_options(&make_credential_request.options ,&map);
                break;
            default:
                break;                
        }
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }

        ret = cbor_value_advance(&map);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }
    }

#if NRF_LOG_DEBUG_CBOR_CONTENT
    NRF_LOG_DEBUG("clientDataHash:");
    NRF_LOG_HEXDUMP_DEBUG(make_credential_request.clientDataHash, CLIENT_DATA_HASH_SIZE);
    NRF_LOG_DEBUG("rp:   id[%s] name[%s]", make_credential_request.rp.id, make_credential_request.rp.name);
    NRF_LOG_DEBUG("user: id[%s] name[%s]", make_credential_request.user.id, make_credential_request.user.name);
    NRF_LOG_DEBUG("publicKeyCredentialTypeName: %s", 
        make_credential_request.cred_param.publicKeyCredentialTypeName);
    NRF_LOG_DEBUG("COSEAlgorithmIdentifier: %d", 
        make_credential_request.cred_param.COSEAlgorithmIdentifier);
    NRF_LOG_DEBUG("options: rk[%d] uv[%d] up[%d]", 
        make_credential_request.options.rk, make_credential_request.options.uv, make_credential_request.options.up);
#endif
    
    return CTAP1_ERR_SUCCESS;
}

bool ctap2_make_credential_is_tup_needed(void)
{
    return (make_credential_request.options.up == 1);
}

// RP IDのSHA-256ハッシュデータを保持
static nrf_crypto_hash_sha256_digest_t rpid_hash;
static size_t                          rpid_hash_size;

static void generate_rpid_hash(void)
{
    // RP IDからSHA-256ハッシュ（32バイト）を生成 
    uint8_t *rpid = make_credential_request.rp.id;
    size_t   rpid_size = strlen((char *)rpid);
    rpid_hash_size = sizeof(rpid_hash);
    fido_crypto_generate_sha256_hash(rpid, rpid_size, rpid_hash, &rpid_hash_size);

#if NRF_LOG_DEBUG_CBOR_CONTENT
    NRF_LOG_DEBUG("RP ID[%s](%d bytes) hash value:", rpid, rpid_size);
    NRF_LOG_HEXDUMP_DEBUG(rpid_hash, rpid_hash_size);
#endif
}

static void generate_pubkey_cred_source(void)
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
    pubkey_cred_source[offset++] = make_credential_request.cred_param.publicKeyCredentialType;

    // Credential private key
    // キーペアを新規生成し、秘密鍵を格納
    fido_crypto_keypair_generate();
    memcpy(pubkey_cred_source + offset, 
        fido_crypto_keypair_private_key(), fido_crypto_keypair_private_key_size());
    offset += fido_crypto_keypair_private_key_size();

    // Relying Party Identifier (size & buffer)
    pubkey_cred_source[offset++] = make_credential_request.rp.id_size;
    memcpy(pubkey_cred_source + offset, 
        make_credential_request.rp.id, make_credential_request.rp.id_size);
    offset += make_credential_request.rp.id_size;
    
    // Public Key Credential Source自体のサイズを、
    // バッファの１バイト目に設定
    pubkey_cred_source[0] = offset;

#if NRF_LOG_DEBUG_CBOR_CONTENT
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

static void generate_credential_id(void)
{
    // Public Key Credential Sourceを編集する
    generate_pubkey_cred_source();

    // Public Key Credential Sourceを
    // AES ECBで暗号化し、
    // credentialIdを生成する
    memset(credential_id, 0x00, sizeof(credential_id));
    fido_crypto_ecb_encrypt(pubkey_cred_source, pubkey_cred_source_block_size, credential_id);
    credential_id_size = pubkey_cred_source_block_size;

#if NRF_LOG_DEBUG_CBOR_CONTENT
    NRF_LOG_DEBUG("credentialId(%d bytes):", credential_id_size);
    NRF_LOG_HEXDUMP_DEBUG(credential_id, credential_id_size);
#endif
}

uint8_t ctap2_make_credential_generate_response_items(void)
{
    // RP IDからrpIdHash（SHA-256ハッシュ）を生成 
    generate_rpid_hash();

    // credentialIdを生成
    generate_credential_id();

    return CTAP1_ERR_SUCCESS;
}
