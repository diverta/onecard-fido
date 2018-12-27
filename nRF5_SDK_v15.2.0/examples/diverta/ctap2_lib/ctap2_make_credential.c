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
} make_credential_request;

// Public Key Credential Sourceを保持
static uint8_t pubkey_cred_source[256];
static size_t  pubkey_cred_source_size;

// 項目のキーに対応する変数
#define MC_clientDataHash       0x01
#define MC_rp                   0x02
#define MC_user                 0x03
#define MC_pubKeyCredParams     0x04
#define MC_excludeList          0x05
#define MC_extensions           0x06
#define MC_options              0x07
#define MC_pinAuth              0x08
#define MC_pinProtocol          0x09

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
        if (key == MC_clientDataHash) {
            ret = parse_fixed_byte_string(&map, make_credential_request.clientDataHash, CLIENT_DATA_HASH_SIZE);
            if (ret == CborNoError) {
                make_credential_request.paramsParsed |= PARAM_clientDataHash;
            }
        }
        if (key == MC_rp) {
            ret = parse_rp(&make_credential_request.rp, &map);
            if (ret == CborNoError) {
                make_credential_request.paramsParsed |= PARAM_rp;
            }
        }
        if (key == MC_user) {
            ret = parse_user(&make_credential_request.user, &map);
            if (ret == CborNoError) {
                make_credential_request.paramsParsed |= PARAM_user;
            }
        }
        if (key == MC_pubKeyCredParams) {
            ret = parse_pub_key_cred_params(&make_credential_request.cred_param, &map);
            if (ret == CborNoError) {
                make_credential_request.paramsParsed |= PARAM_pubKeyCredParams;
            }
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
#endif
    
    return CTAP1_ERR_SUCCESS;
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
    int offset = 0;
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
    
    // サイズを設定
    pubkey_cred_source_size = offset;

#if NRF_LOG_DEBUG_CBOR_CONTENT
    NRF_LOG_DEBUG("Public Key Credential Source(%d bytes):", pubkey_cred_source_size);
    NRF_LOG_HEXDUMP_DEBUG(pubkey_cred_source, pubkey_cred_source_size);
#endif
}

static void generate_credential_id(void)
{
    // Public Key Credential Sourceを編集する
    generate_pubkey_cred_source();
    
    // TODO:
    // Public Key Credential Sourceを
    // AES ECBで暗号化し、
    // credentialIdを生成する    
}

uint8_t ctap2_make_credential_generate_response_items(void)
{
    // RP IDからrpIdHash（SHA-256ハッシュ）を生成 
    generate_rpid_hash();

    // credentialIdを生成
    generate_credential_id();

    return CTAP1_ERR_SUCCESS;
}
