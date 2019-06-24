/* 
 * File:   ctap2_get_assertion.c
 * Author: makmorit
 *
 * Created on 2019/01/03, 11:05
 */
#include "cbor.h"
#include "ctap2_common.h"
#include "ctap2_client_pin_token.h"
#include "ctap2_cbor_parse.h"
#include "ctap2_pubkey_credential.h"
#include "ctap2_extension_hmac_secret.h"
#include "fido_common.h"
#include "fido_crypto.h"
#include "fido_log.h"

// for u2f_flash_token_counter
#include "fido_flash.h"

// for u2f_crypto_signature_data
#include "u2f_signature.h"

// for debug cbor data
#define LOG_DEBUG_CLHASH_DATA_BUFF  false
#define LOG_HEXDUMP_DEBUG_CBOR      false
#define LOG_DEBUG_CBOR_REQUEST      false
#define LOG_DEBUG_ALLOW_LIST        false
#define LOG_DEBUG_AUTH_DATA_ITEMS   false
#define LOG_DEBUG_AUTH_DATA_BUFF    false
#define LOG_DEBUG_SIGN_BUFF         false
#define LOG_DEBUG_CBOR_RESPONSE     false
#define LOG_DEBUG_PIN_AUTH          false

// デコードされた
// authenticatorGetAssertion
// リクエストデータを保持する構造体
struct {
    uint8_t                  clientDataHash[CLIENT_DATA_HASH_SIZE];
    CTAP_RP_ID_T             rp;
    CTAP_OPTIONS_T           options;
    CTAP_ALLOW_LIST_T        allowList;
    uint8_t                  pinAuth[PIN_AUTH_SIZE];
    uint8_t                  pinProtocol;
    CTAP_EXTENSIONS_T        extensions;
} ctap2_request;


static void debug_decoded_request()
{
#if LOG_DEBUG_CLHASH_DATA_BUFF
    fido_log_debug("clientDataHash:");
    fido_log_print_hexdump_debug(ctap2_request.clientDataHash, CLIENT_DATA_HASH_SIZE);
#endif

#if LOG_DEBUG_CBOR_REQUEST
    fido_log_debug("rp id[%s]", ctap2_request.rp.id);
    //fido_log_debug("user: id[%s] name[%s]", make_credential_request.user.id, make_credential_request.user.name);
    //fido_log_debug("publicKeyCredentialTypeName: %s", 
    //    make_credential_request.cred_param.publicKeyCredentialTypeName);
    //fido_log_debug("COSEAlgorithmIdentifier: %d", 
    //    make_credential_request.cred_param.COSEAlgorithmIdentifier);
#endif

#if LOG_DEBUG_ALLOW_LIST
    int x;
    CTAP_CREDENTIAL_DESC_T *desc;
    for (x = 0; x < ctap2_request.allowListSize; x++) {
        desc = &ctap2_request.allowList[x];
        fido_log_debug("allowList[%d]: type[%d]", x, desc->type);
        fido_log_print_hexdump_debug(desc->credential_id, desc->credential_id_size);
    }
#endif

#if LOG_DEBUG_CBOR_REQUEST
    fido_log_debug("options: rk[%d] uv[%d] up[%d]", 
        ctap2_request.options.rk, ctap2_request.options.uv, ctap2_request.options.up);
#endif

#if LOG_DEBUG_PIN_AUTH
    fido_log_debug("pinAuth (pinProtocol=0x%02x):", ctap2_request.pinProtocol);
    fido_log_print_hexdump_debug(ctap2_request.pinAuth, PIN_AUTH_SIZE);
#endif
}

uint8_t ctap2_get_assertion_decode_request(uint8_t *cbor_data_buffer, size_t cbor_data_length)
{
    CborParser  parser;
    CborValue   it;
    CborValue   map;
    size_t      map_length;
    CborType    type;
    CborError   ret;
    uint8_t     i;
    int         key;
    int         intval;

#if LOG_HEXDUMP_DEBUG_CBOR
    fido_log_debug("authenticatorGetAssertion request cbor(%d bytes):", cbor_data_length);
    int j, k;
    int max = (cbor_data_length < 288) ? cbor_data_length : 288;
    for (j = 0; j < max; j += 64) {
        k = max - j;
        fido_log_print_hexdump_debug(cbor_data_buffer + j, (k < 64) ? k : 64);
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
            case 4:
                // extensions (CBOR map)
                ret = parse_extensions(&map, &ctap2_request.extensions);
                if (ret != CTAP1_ERR_SUCCESS) {
                    return ret;
                }
                break;
            case 5:
                // options (Map of authenticator options)
                ret = parse_options(&ctap2_request.options, &map, false);
                if (ret != CTAP1_ERR_SUCCESS) {
                    return ret;
                }
                break;
            case 6:
                // pinAuth（Byte Array）
                ret = parse_fixed_byte_string(&map, ctap2_request.pinAuth, PIN_AUTH_SIZE);
                if (ret != CTAP1_ERR_SUCCESS) {
                    return ret;
                }
                break;
            case 7:
                // pinProtocol (Unsigned Integer)
                if (cbor_value_get_type(&map) != CborIntegerType) {
                    return CTAP2_ERR_INVALID_CBOR_TYPE;
                }
                ret = cbor_value_get_int_checked(&map, &intval);
                if (ret != CborNoError) {
                    return CTAP2_ERR_CBOR_PARSING;
                }
                ctap2_request.pinProtocol = (uint8_t)intval;
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
    if (ctap2_pubkey_credential_source_hash_size() != SHA_256_HASH_SIZE) {
        return CTAP1_ERR_OTHER;
    }

    // Public Key Credential Sourceから
    // 生成されたSHA-256ハッシュ値をキーとし、
    // トークンカウンターレコードを検索
    uint8_t *p_hash = ctap2_pubkey_credential_source_hash();
    if (fido_flash_token_counter_read(p_hash) == false) {
        // 紐づくトークンカウンターがない場合は
        // エラーレスポンスを生成して戻す
        fido_log_error("sign counter not found");
        return CTAP2_ERR_NO_CREDENTIALS;
    }

    // CTAP2クライアントから受信したrpIdHashと、
    // トークンカウンターレコードに紐づくrpIdHashを比較
    char *ctap2_rpid_hash = (char *)ctap2_generated_rpid_hash();
    char *hash_for_check = (char *)fido_flash_token_counter_get_check_hash();
    if (strncmp(ctap2_rpid_hash, hash_for_check, 32) != 0) {
        // 紐づくトークンカウンターがない場合は
        // エラーレスポンスを生成して戻す
        fido_log_error("unavailable sign counter");
        return CTAP2_ERR_NO_CREDENTIALS;
    }
    fido_log_debug("sign counter found (value=%d)", fido_flash_token_counter_value());

    // +1 してctap2_sign_countに設定
    uint32_t ctap2_sign_count = fido_flash_token_counter_value();
    ctap2_set_sign_count(++ctap2_sign_count);

    return CTAP1_ERR_SUCCESS;
}

static uint8_t add_extensions_cbor(uint8_t *authenticator_data)
{
    // レスポンス用CBORを生成
    if (ctap2_extension_hmac_secret_cbor_for_get(
        &ctap2_request.extensions) != CTAP1_ERR_SUCCESS) {
        return 0;
    }

    // authenticatorData領域に格納
    memcpy(authenticator_data, ctap2_extension_hmac_secret_cbor(), ctap2_extension_hmac_secret_cbor_size());
    return ctap2_extension_hmac_secret_cbor_size();
}

static void generate_authenticator_data(void)
{
    // rpIdHashの先頭アドレスとサイズを取得
    uint8_t *ctap2_rpid_hash = ctap2_generated_rpid_hash();
    size_t   ctap2_rpid_hash_size = ctap2_generated_rpid_hash_size();

    // extensions設定時はflagsを追加設定
    //   Extension data included (0x80)
    if (ctap2_request.extensions.hmac_secret.hmac_secret_parsed) {
        ctap2_flags_set(0x80);
    }

    // Authenticator data各項目を
    // 先頭からバッファにセット
    //  rpIdHash
    uint8_t offset = 0;
    memset(authenticator_data, 0x00, sizeof(authenticator_data));
    memcpy(authenticator_data + offset, ctap2_rpid_hash, ctap2_rpid_hash_size);
    offset += ctap2_rpid_hash_size;
    //  flags
    authenticator_data[offset++] = ctap2_flags_value();
    //  signCount
    fido_set_uint32_bytes(authenticator_data + offset, ctap2_current_sign_count());
    offset += sizeof(uint32_t);
    //  extensions設定時
    //    {"hmac-secret": AES256-CBC(sharedSecret, IV=0, output1 (32 bytes) || output2 (32 bytes))}
    if (ctap2_request.extensions.hmac_secret.hmac_secret_parsed) {
        offset += add_extensions_cbor(authenticator_data + offset);
    }

#if LOG_DEBUG_AUTH_DATA_BUFF
    int j, k;
    fido_log_debug("Authenticator data(%d bytes):", offset);
    for (j = 0; j < offset; j += 64) {
        k = offset - j;
        fido_log_print_hexdump_debug(authenticator_data + j, (k < 64) ? k : 64);
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

#if LOG_DEBUG_SIGN_BUFF
    fido_log_debug("Signature(%d bytes):", u2f_signature_data_size());
    fido_log_print_hexdump_debug(u2f_signature_data_buffer(), u2f_signature_data_size());
#endif

    return CTAP1_ERR_SUCCESS;
}

uint8_t ctap2_get_assertion_generate_response_items(void)
{
    // RP IDからrpIdHash（SHA-256ハッシュ）を生成 
    uint8_t *rpid = ctap2_request.rp.id;
    size_t   rpid_size = strlen((char *)rpid);
    ctap2_generate_rpid_hash(rpid, rpid_size);

#if LOG_DEBUG_AUTH_DATA_ITEMS
    fido_log_debug("RP ID[%s](%d bytes) hash value:", rpid, rpid_size);
    fido_log_print_hexdump_debug(ctap2_rpid_hash, ctap2_rpid_hash_size);
#endif

    // flags編集
    //   User Present result (0x01)
    ctap2_flags_set(0x01);

    // 秘密鍵とCredential Source Hash
    // （トークンカウンターのキー）を
    // credentialIdから取出し
    uint8_t ret = ctap2_pubkey_credential_restore_private_key(&ctap2_request.allowList);
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
    CborError ret = cbor_encode_int(map, 0x01);
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
    CborError   ret;
    ret = cbor_encoder_create_map(&encoder, &map, 4);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    // Credential (0x01: RESP_credential)
    ret = add_credential_descriptor(&map, ctap2_pubkey_credential_restored_id());
    if (ret != CborNoError) {
        return ret;
    }

    // authData (0x02: RESP_authData)
    ret = cbor_encode_int(&map, 0x02);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = cbor_encode_byte_string(&map, authenticator_data, authenticator_data_size);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    // signature (0x03: RESP_signature)
    ret = cbor_encode_int(&map, 0x03);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = cbor_encode_byte_string(&map, 
        u2f_signature_data_buffer(), u2f_signature_data_size());
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    // numberOfCredentials (0x05: RESP_numberOfCredentials)
    ret = cbor_encode_int(&map, 0x05);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = cbor_encode_int(&map, ctap2_pubkey_credential_number());
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    ret = cbor_encoder_close_container(&encoder, &map);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    // CBORバッファの長さを設定
    *encoded_buff_size = cbor_encoder_get_buffer_size(&encoder, encoded_buff);

#if LOG_DEBUG_CBOR_RESPONSE
    fido_log_debug("authenticatorGetAssertion response(%d bytes):", *encoded_buff_size);
    int j, k;
    int max = (*encoded_buff_size) < 256 ? (*encoded_buff_size) : 256;
    for (j = 0; j < max; j += 64) {
        k = max - j;
        fido_log_print_hexdump_debug(encoded_buff + j, (k < 64) ? k : 64);
    }
#endif

    return CTAP1_ERR_SUCCESS;
}

uint8_t ctap2_get_assertion_update_token_counter(void)
{
    // 例外抑止
    if (ctap2_pubkey_credential_source_hash_size() != SHA_256_HASH_SIZE) {
        return CTAP1_ERR_OTHER;
    }

    // Public Key Credential Sourceから
    // 生成されたSHA-256ハッシュ値をキーとし、
    // トークンカウンターレコードを更新
    uint8_t *p_hash = ctap2_pubkey_credential_source_hash();
    uint8_t *p_hash_for_check = ctap2_generated_rpid_hash();
    if (fido_flash_token_counter_write(p_hash, ctap2_current_sign_count(), p_hash_for_check) == false) {
        // NGであれば終了
        return CTAP1_ERR_OTHER;
    }

    // 後続のレスポンス生成・送信は、
    // Flash ROM書込み完了後に行われる
    fido_log_debug("sign counter updated (value=%d)", ctap2_current_sign_count());
    return CTAP1_ERR_SUCCESS;
}

uint8_t ctap2_get_assertion_verify_pin_auth(void)
{
    // PIN認証が必要でない場合は終了
    if (ctap2_request.pinProtocol != 0x01) {
        return CTAP1_ERR_SUCCESS;
    }

    // pinAuthの妥当性チェックを行い、
    // NGの場合はPIN認証失敗
    uint8_t ctap2_status = ctap2_client_pin_token_verify_pin_auth(ctap2_request.clientDataHash, ctap2_request.pinAuth);
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        fido_log_error("pinAuth verification failed");
        return ctap2_status;
    }

    // flagsを設定
    //   User Verified result (0x04)
    ctap2_flags_set(0x04);
    fido_log_debug("pinAuth verification success");
    return CTAP1_ERR_SUCCESS;
}
