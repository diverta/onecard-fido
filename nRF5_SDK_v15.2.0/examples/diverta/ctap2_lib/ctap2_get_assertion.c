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
#include "ctap2_pubkey_credential.h"
#include "fido_common.h"

// for u2f_flash_token_counter
#include "fido_flash.h"

// for u2f_crypto_signature_data
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
#define NRF_LOG_DEBUG_AUTH_DATA_ITEMS   false
#define NRF_LOG_DEBUG_AUTH_DATA_BUFF    false
#define NRF_LOG_DEBUG_SIGN_BUFF         false
#define NRF_LOG_DEBUG_CBOR_RESPONSE     false

// デコードされた
// authenticatorGetAssertion
// リクエストデータを保持する構造体
struct {
    uint8_t                  clientDataHash[CLIENT_DATA_HASH_SIZE];
    CTAP_RP_ID_T             rp;
    CTAP_OPTIONS_T           options;
    CTAP_ALLOW_LIST_T        allowList;
} ctap2_request;


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
    // 必須項目チェック済みフラグを初期化
    uint8_t must_item_flag = 0;

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

        switch(key) {
            case 1:
                // rpId
                ret = parse_rp_id(&ctap2_request.rp, &map);
                if (ret != CTAP1_ERR_SUCCESS) {
                    return ret;
                }
                must_item_flag |= 0x01;
                break;
            case 2:
                // clientDataHash (Byte Array)
                ret = parse_fixed_byte_string(&map, ctap2_request.clientDataHash, CLIENT_DATA_HASH_SIZE);
                if (ret != CTAP1_ERR_SUCCESS) {
                    return ret;
                }
                must_item_flag |= 0x02;
                break;
            case 3:
                // allowList
                ret = parse_allow_list(&ctap2_request.allowList, &map);
                if (ret != CTAP1_ERR_SUCCESS) {
                    return ret;
                }
                break;
            case 5:
                // options (Map of authenticator options)
                ret = parse_options(&ctap2_request.options, &map);
                if (ret != CTAP1_ERR_SUCCESS) {
                    return ret;
                }
                break;
            default:
                break;                
        }

        ret = cbor_value_advance(&map);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }
    }

    debug_decoded_request();

    // 必須項目が揃っていない場合はエラー
    if (must_item_flag != 0x03) {
        return CTAP2_ERR_MISSING_PARAMETER;
    }

    return CTAP1_ERR_SUCCESS;
}

bool ctap2_get_assertion_is_tup_needed(void)
{
    return (ctap2_request.options.up == 1);
}

static uint8_t read_token_counter(void)
{
    // 例外抑止
    if (ctap2_pubkey_credential_source_hash_size() != sizeof(nrf_crypto_hash_sha256_digest_t)) {
        return CTAP2_ERR_PROCESSING;
    }

    // Public Key Credential Sourceから
    // 生成されたSHA-256ハッシュ値をキーとし、
    // トークンカウンターレコードを検索
    uint8_t *p_hash = ctap2_pubkey_credential_source_hash();
    if (fido_flash_token_counter_read(p_hash) == false) {
        // 紐づくトークンカウンターがない場合は
        // エラーレスポンスを生成して戻す
        NRF_LOG_ERROR("sign counter not found");
        return CTAP2_ERR_PROCESSING;
    }
    NRF_LOG_DEBUG("sign counter found (value=%d)", fido_flash_token_counter_value());

    // +1 してctap2_sign_countに設定
    ctap2_sign_count = fido_flash_token_counter_value();
    ctap2_sign_count++;

    return CTAP1_ERR_SUCCESS;
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

static uint8_t generate_sign(void)
{
    // 署名を実行
    if (ctap2_generate_signature(
        ctap2_request.clientDataHash, ctap2_pubkey_credential_private_key()) == false) {
        return CTAP2_ERR_VENDOR_FIRST;
    }

#if NRF_LOG_DEBUG_SIGN_BUFF
    NRF_LOG_DEBUG("Signature(%d bytes):", u2f_crypto_signature_data_size());
    NRF_LOG_HEXDUMP_DEBUG(u2f_crypto_signature_data_buffer(), u2f_crypto_signature_data_size());
#endif

    return CTAP1_ERR_SUCCESS;
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

    // 秘密鍵とCredential Source Hash
    // （トークンカウンターのキー）を
    // credentialIdから取出し
    uint8_t ret = ctap2_pubkey_credential_restore_private_key(&ctap2_request.allowList, &ctap2_request.rp);
    if (ret != CTAP1_ERR_SUCCESS) {
        return ret;
    }

    // トークンカウンターからsign countを取得し、
    // あらかじめ +1 しておく
    ret = read_token_counter();
    if (ret != CTAP1_ERR_SUCCESS) {
        return ret;
    }
    
    // Authenticator dataを生成
    generate_authenticator_data();

    // credentialIdから取り出した
    // 秘密鍵により署名を生成
    ret = generate_sign();
    if (ret != CTAP1_ERR_SUCCESS) {
        return ret;
    }
    
    return CTAP1_ERR_SUCCESS;
}

static uint8_t add_credential_descriptor(CborEncoder *map, CTAP_CREDENTIAL_DESC_T *cred)
{
    // Credential (0x01: RESP_credential)
    CborEncoder desc;
    int ret = cbor_encode_int(map, 0x01);
    if (ret != CborNoError) {
        return ret;
    }

    ret = cbor_encoder_create_map(map, &desc, 2);
    if (ret == CborNoError) {
        ret = cbor_encode_text_string(&desc, "type", 4);
        if (ret != CborNoError) {
            return ret;
        }

        ret = cbor_encode_text_string(&desc, "public-key", 10);
        if (ret != CborNoError) {
            return ret;
        }

        ret = cbor_encode_text_string(&desc, "id", 2);
        if (ret != CborNoError) {
            return ret;
        }

        ret = cbor_encode_byte_string(&desc, (uint8_t*)&cred->credential_id, cred->credential_id_size);
        if (ret != CborNoError) {
            return ret;
        }
    }

    ret = cbor_encoder_close_container(map, &desc);
    if (ret != CborNoError) {
        return ret;
    }

    return CborNoError;
}

uint8_t ctap2_get_assertion_encode_response(uint8_t *encoded_buff, size_t *encoded_buff_size)
{
    // CBORエンコーダーを初期化
    CborEncoder encoder;
    cbor_encoder_init(&encoder, encoded_buff, *encoded_buff_size, 0);

    // Map初期化
    CborEncoder map;
    int         ret;
    ret = cbor_encoder_create_map(&encoder, &map, 4);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }

    // Credential (0x01: RESP_credential)
    ret = add_credential_descriptor(&map, ctap2_pubkey_credential_restored_id());
    if (ret != CborNoError) {
        return ret;
    }

    // authData (0x02: RESP_authData)
    ret = cbor_encode_int(&map, 0x02);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }
    ret = cbor_encode_byte_string(&map, authenticator_data, authenticator_data_size);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }

    // signature (0x03: RESP_signature)
    ret = cbor_encode_int(&map, 0x03);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }
    ret = cbor_encode_byte_string(&map, 
        u2f_crypto_signature_data_buffer(), u2f_crypto_signature_data_size());
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }

    // numberOfCredentials (0x05: RESP_numberOfCredentials)
    ret = cbor_encode_int(&map, 0x05);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }
    ret = cbor_encode_int(&map, ctap2_pubkey_credential_number());
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }

    ret = cbor_encoder_close_container(&encoder, &map);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }

    // CBORバッファの長さを設定
    *encoded_buff_size = cbor_encoder_get_buffer_size(&encoder, encoded_buff);

#if NRF_LOG_DEBUG_CBOR_RESPONSE
    NRF_LOG_DEBUG("authenticatorGetAssertion response(%d bytes):", *encoded_buff_size);
    int j, k;
    int max = (*encoded_buff_size) < 256 ? (*encoded_buff_size) : 256;
    for (j = 0; j < max; j += 64) {
        k = max - j;
        NRF_LOG_HEXDUMP_DEBUG(encoded_buff + j, (k < 64) ? k : 64);
    }
#endif

    return CTAP1_ERR_SUCCESS;
}

uint8_t ctap2_get_assertion_update_token_counter(void)
{
    // 例外抑止
    if (ctap2_pubkey_credential_source_hash_size() != sizeof(nrf_crypto_hash_sha256_digest_t)) {
        return CTAP2_ERR_PROCESSING;
    }

    // Public Key Credential Sourceから
    // 生成されたSHA-256ハッシュ値をキーとし、
    // トークンカウンターレコードを更新
    uint8_t *p_hash = ctap2_pubkey_credential_source_hash();
    uint32_t reserve_word = 0xffffffff;
    if (fido_flash_token_counter_write(p_hash, ctap2_sign_count, reserve_word) == false) {
        // NGであれば終了
        return CTAP2_ERR_PROCESSING;
    }

    // 後続のレスポンス生成・送信は、
    // Flash ROM書込み完了後に行われる
    NRF_LOG_DEBUG("sign counter updated (value=%d)", ctap2_sign_count);
    return CTAP1_ERR_SUCCESS;
}
