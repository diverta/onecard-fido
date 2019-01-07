/* 
 * File:   ctap2_get_assertion.c
 * Author: makmorit
 *
 * Created on 2019/01/03, 11:05
 */
#include "sdk_common.h"

#include "cbor.h"
#include "ctap2_common.h"
#include "ctap2_cbor_parse.h"
#include "fido_common.h"
#include "fido_crypto_ecb.h"

// for u2f_flash_token_counter
#include "u2f_flash.h"

// for u2f_crypto_sign & other
#include "u2f_crypto.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ctap2_get_assertion
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for debug cbor data
#define NRF_LOG_DEBUG_CLHASH_DATA_BUFF  false
#define NRF_LOG_HEXDUMP_DEBUG_CBOR      false
#define NRF_LOG_DEBUG_CBOR_REQUEST      false
#define NRF_LOG_DEBUG_ALLOW_LIST        false
#define NRF_LOG_DEBUG_AUTH_DATA_ITEMS   true
#define NRF_LOG_DEBUG_AUTH_DATA_BUFF    true
#define NRF_LOG_DEBUG_SIGN_BUFF         false
#define NRF_LOG_DEBUG_CBOR_RESPONSE     false

// デコードされた
// authenticatorGetAssertion
// リクエストデータを保持する構造体
struct {
    bool                     clientDataHashPresent;
    uint8_t                  clientDataHash[CLIENT_DATA_HASH_SIZE];
    CTAP_RP_ID_T             rp;
    CTAP_OPTIONS_T           options;
    bool                     allowListPresent;
    uint8_t                  allowListSize;
    CTAP_CREDENTIAL_DESC_T   allowList[ALLOW_LIST_MAX_SIZE];
} ctap2_request;

// credential IDから取り出した秘密鍵の
// 格納領域を保持
uint8_t *private_key_be;


static void debug_decoded_request()
{
#if NRF_LOG_DEBUG_CLHASH_DATA_BUFF
    NRF_LOG_DEBUG("clientDataHash:");
    NRF_LOG_HEXDUMP_DEBUG(ctap2_request.clientDataHash, CLIENT_DATA_HASH_SIZE);
#endif

#if NRF_LOG_DEBUG_CBOR_REQUEST
    NRF_LOG_DEBUG("rp id[%s]", ctap2_request.rp.id);
    //NRF_LOG_DEBUG("user: id[%s] name[%s]", make_credential_request.user.id, make_credential_request.user.name);
    //NRF_LOG_DEBUG("publicKeyCredentialTypeName: %s", 
    //    make_credential_request.cred_param.publicKeyCredentialTypeName);
    //NRF_LOG_DEBUG("COSEAlgorithmIdentifier: %d", 
    //    make_credential_request.cred_param.COSEAlgorithmIdentifier);
#endif

#if NRF_LOG_DEBUG_ALLOW_LIST
    int x;
    CTAP_CREDENTIAL_DESC_T *desc;
    for (x = 0; x < ctap2_request.allowListSize; x++) {
        desc = &ctap2_request.allowList[x];
        NRF_LOG_DEBUG("allowList[%d]: type[%d]", x, desc->type);
        NRF_LOG_HEXDUMP_DEBUG(desc->credential_id, desc->credential_id_size);
    }
#endif

#if NRF_LOG_DEBUG_CBOR_REQUEST
    NRF_LOG_DEBUG("options: rk[%d] uv[%d] up[%d]", 
        ctap2_request.options.rk, ctap2_request.options.uv, ctap2_request.options.up);
#endif
}

uint8_t ctap2_get_assertion_decode_request(uint8_t *cbor_data_buffer, size_t cbor_data_length)
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
    NRF_LOG_DEBUG("authenticatorGetAssertion request cbor(%d bytes):", cbor_data_length);
    int j, k;
    int max = (cbor_data_length < 288) ? cbor_data_length : 288;
    for (j = 0; j < max; j += 64) {
        k = max - j;
        NRF_LOG_HEXDUMP_DEBUG(cbor_data_buffer + j, (k < 64) ? k : 64);
    }
#endif

    // リクエスト格納領域初期化
    memset(&ctap2_request, 0x00, sizeof(ctap2_request));

    // user presenceのデフォルトを
    // true に設定しておく。
    ctap2_request.options.up = true;

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
            return CTAP2_ERR_INVALID_CBOR_TYPE;
        }
        ret = cbor_value_get_int_checked(&map, &key);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }
        ret = cbor_value_advance(&map);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }

        ret = CborNoError;
        switch(key) {
            case 1:
                // rpId
                ret = parse_rp_id(&ctap2_request.rp, &map);
                break;
            case 2:
                // clientDataHash (Byte Array)
                ret = parse_fixed_byte_string(&map, ctap2_request.clientDataHash, CLIENT_DATA_HASH_SIZE);
                ctap2_request.clientDataHashPresent = (ret == CborNoError);
                break;
            case 3:
                // allowList
                ret = parse_allow_list(ctap2_request.allowList, &ctap2_request.allowListSize, &map);
                ctap2_request.allowListPresent = (ret == CborNoError);
                break;
            case 5:
                // options (Map of authenticator options)
                ret = parse_options(&ctap2_request.options, &map);
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

    debug_decoded_request();
    return CTAP1_ERR_SUCCESS;
}

bool ctap2_get_assertion_is_tup_needed(void)
{
    return (ctap2_request.options.up == 1);
}

static void generate_authenticator_data(void)
{
    // Authenticator data各項目を
    // 先頭からバッファにセット
    //  rpIdHash
    int offset = 0;
    memset(authenticator_data, 0x00, sizeof(authenticator_data));
    memcpy(authenticator_data + offset, ctap2_rpid_hash, ctap2_rpid_hash_size);
    offset += ctap2_rpid_hash_size;
    //  flags
    authenticator_data[offset++] = ctap2_flags;
    //  signCount
    fido_set_uint32_bytes(authenticator_data + offset, ctap2_sign_count);
    offset += sizeof(uint32_t);

#if NRF_LOG_DEBUG_AUTH_DATA_BUFF
    int j, k;
    NRF_LOG_DEBUG("Authenticator data(%d bytes):", offset);
    for (j = 0; j < offset; j += 64) {
        k = offset - j;
        NRF_LOG_HEXDUMP_DEBUG(authenticator_data + j, (k < 64) ? k : 64);
    }
#endif

    // データ長を設定
    authenticator_data_size = offset;
}

static void decrypto_credential_id(uint8_t *credential_id, size_t credential_id_size)
{
    // authenticatorGetAssertionリクエストから取得した
    // credentialIdを復号化
    memset(pubkey_cred_source, 0, sizeof(pubkey_cred_source));
    fido_crypto_ecb_decrypt(credential_id, credential_id_size, pubkey_cred_source);

#if NRF_LOG_DEBUG_AUTH_DATA_ITEMS
        NRF_LOG_DEBUG("Public Key Credential Source(%d bytes):", pubkey_cred_source[0]);
        NRF_LOG_HEXDUMP_DEBUG(pubkey_cred_source, pubkey_cred_source[0]);
#endif
}

static bool get_private_key_from_credential_id(void)
{
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
    size_t request_rp_id_size = ctap2_request.rp.id_size;
    char  *request_rp_id = (char *)ctap2_request.rp.id;

    // リクエストされたrpIdと、Public Key Credential SourceのrpIdを比較し、
    // 一致している場合は true
    src_rp_id_size = (src_rp_id_size < request_rp_id_size) ? src_rp_id_size : request_rp_id_size;
    if (strncmp(request_rp_id, src_rp_id, src_rp_id_size) == 0) {
#if NRF_LOG_DEBUG_AUTH_DATA_ITEMS
        private_key_be = pubkey_cred_source + 2;
        NRF_LOG_DEBUG("Private key of RP[%s]:", src_rp_id);
        NRF_LOG_HEXDUMP_DEBUG(private_key_be, 32);
#endif
        return true;

    } else {
        private_key_be = NULL;
        return false;
    }
}

static uint8_t restore_private_key(void)
{
    int x;
    CTAP_CREDENTIAL_DESC_T *desc;

    // credentialIdリストの先頭から逐一処理
    for (x = 0; x < ctap2_request.allowListSize; x++) {
        // credentialIdをAES ECBで復号化し、
        // Public Key Credential Sourceを取得
        desc = &ctap2_request.allowList[x];
        decrypto_credential_id(desc->credential_id, desc->credential_id_size);

        // rpIdをマッチングさせ、一致していれば秘密鍵を取り出す
        if (get_private_key_from_credential_id()) {
            return CTAP1_ERR_SUCCESS;
        }
    }

    // credentialIdリストに
    // 一致するrpIdがない場合はエラー
    return CTAP2_ERR_PROCESSING;
}

static uint8_t generate_sign(void)
{
    // 秘密鍵をcredentialIdから取出し
    uint8_t ret = restore_private_key();
    if (ret != CTAP1_ERR_SUCCESS) {
        return ret;
    }
    
    // TODO: 仮の実装です。
    return CTAP2_ERR_PROCESSING;
}

uint8_t ctap2_get_assertion_generate_response_items(void)
{
    // RP IDからrpIdHash（SHA-256ハッシュ）を生成 
    uint8_t *rpid = ctap2_request.rp.id;
    size_t   rpid_size = strlen((char *)rpid);
    ctap2_generate_rpid_hash(rpid, rpid_size);

#if NRF_LOG_DEBUG_AUTH_DATA_ITEMS
    NRF_LOG_DEBUG("RP ID[%s](%d bytes) hash value:", rpid, rpid_size);
    NRF_LOG_HEXDUMP_DEBUG(ctap2_rpid_hash, ctap2_rpid_hash_size);
#endif

    // flags編集
    //   User Present result (0x01)
    ctap2_flags = 0x01;

    // sign countを取得し、
    // rpIdHashに紐づくトークンカウンターを検索
    if (u2f_flash_token_counter_read(ctap2_rpid_hash) == false) {
        // appIdHashに紐づくトークンカウンターがない場合は
        // エラーレスポンスを生成して戻す
        NRF_LOG_ERROR("sign counter not found");
        return CTAP2_ERR_PROCESSING;
    }
    NRF_LOG_DEBUG("sign counter found (value=%d)", u2f_flash_token_counter_value());

    // +1 してctap2_sign_countに設定
    ctap2_sign_count = u2f_flash_token_counter_value();
    ctap2_sign_count++;
    
    // Authenticator dataを生成
    generate_authenticator_data();

    // credentialIdから取り出した
    // 秘密鍵により署名を生成
    uint8_t ret = generate_sign();
    if (ret != CTAP1_ERR_SUCCESS) {
        return ret;
    }
    
    return CTAP1_ERR_SUCCESS;
}

uint8_t ctap2_get_assertion_encode_response(uint8_t *encoded_buff, size_t *encoded_buff_size)
{
    // TODO: 仮の実装です。
    return CTAP2_ERR_PROCESSING;
}

uint8_t ctap2_get_assertion_update_token_counter(void)
{
    // TODO: 仮の実装です。
    return CTAP2_ERR_PROCESSING;
}
