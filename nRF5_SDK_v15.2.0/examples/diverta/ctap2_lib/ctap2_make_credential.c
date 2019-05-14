/* 
 * File:   ctap2_make_credential.c
 * Author: makmorit
 *
 * Created on 2018/12/25, 11:33
 */
#include "sdk_common.h"

#include "cbor.h"
#include "ctap2_common.h"
#include "ctap2_cbor_authgetinfo.h"
#include "ctap2_cbor_parse.h"
#include "ctap2_client_pin_token.h"
#include "ctap2_pubkey_credential.h"
#include "fido_common.h"
#include "fido_crypto_keypair.h"

// for u2f_flash_keydata_read & u2f_flash_keydata_available
#include "fido_flash.h"

// for u2f_crypto_signature_data
#include "u2f_crypto.h"

// for u2f_securekey_skey_be
#include "u2f_register.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ctap2_make_credential
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for debug cbor data
#define NRF_LOG_DEBUG_CLHASH_DATA_BUFF  false
#define NRF_LOG_HEXDUMP_DEBUG_CBOR      false
#define NRF_LOG_DEBUG_CBOR_REQUEST      false
#define NRF_LOG_DEBUG_AUTH_DATA_ITEMS   false
#define NRF_LOG_DEBUG_AUTH_DATA_BUFF    false
#define NRF_LOG_DEBUG_SIGN_BUFF         false
#define NRF_LOG_DEBUG_CBOR_RESPONSE     false
#define NRF_LOG_DEBUG_PIN_AUTH          false

// デコードされた
// authenticatorMakeCredential
// リクエストデータを保持する構造体
struct {
    uint8_t                  clientDataHash[CLIENT_DATA_HASH_SIZE];
    CTAP_RP_ID_T             rp;
    CTAP_USER_ENTITY_T       user;
    CTAP_PUBKEY_CRED_PARAM_T cred_param;
    CTAP_OPTIONS_T           options;
    uint8_t                  pinAuth[PIN_AUTH_SIZE];
    uint8_t                  pinProtocol;
    CTAP_EXTENSIONS_T        extensions;
} ctap2_request;

// credentialPublicKeyを保持
static uint8_t credential_pubkey[CREDENTIAL_ID_MAX_SIZE];
static size_t  credential_pubkey_size;

uint8_t ctap2_make_credential_decode_request(uint8_t *cbor_data_buffer, size_t cbor_data_length)
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

#if NRF_LOG_HEXDUMP_DEBUG_CBOR
    NRF_LOG_DEBUG("authenticatorMakeCredential request cbor(%d bytes):", cbor_data_length);
    int j, k;
    int max = (cbor_data_length < 288) ? cbor_data_length : 288;
    for (j = 0; j < max; j += 64) {
        k = max - j;
        NRF_LOG_HEXDUMP_DEBUG(cbor_data_buffer + j, (k < 64) ? k : 64);
    }
#else
    UNUSED_PARAMETER(cbor_data_buffer);
    UNUSED_PARAMETER(cbor_data_length);
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

        switch(key) {
            case 1:
                // clientDataHash (Byte Array)
                ret = parse_fixed_byte_string(&map, ctap2_request.clientDataHash, CLIENT_DATA_HASH_SIZE);
                if (ret != CTAP1_ERR_SUCCESS) {
                    return ret;
                }
                must_item_flag |= 0x01;
                break;
            case 2:
                // rp (PublicKeyCredentialRpEntity)
                ret = parse_rp(&ctap2_request.rp, &map);
                if (ret != CborNoError) {
                    return CTAP2_ERR_CBOR_PARSING;
                }
                must_item_flag |= 0x02;
                break;
            case 3:
                // user (PublicKeyCredentialUserEntity)
                ret = parse_user(&ctap2_request.user, &map);
                if (ret != CborNoError) {
                    return CTAP2_ERR_CBOR_PARSING;
                }
                must_item_flag |= 0x04;
                break;
            case 4:
                // pubKeyCredParams (CBOR Array)
                ret = parse_pub_key_cred_params(&ctap2_request.cred_param, &map);
                if (ret != CTAP1_ERR_SUCCESS) {
                    return ret;
                }
                must_item_flag |= 0x08;
                break;
            case 5:
                // excludeList (Sequence)
                ret = parse_verify_exclude_list(&map);
                if (ret != CTAP1_ERR_SUCCESS) {
                    return ret;
                }
                break;
            case 6:
                // extensions (CBOR map)
                ret = parse_extensions(&map, &ctap2_request.extensions);
                if (ret != CTAP1_ERR_SUCCESS) {
                    return ret;
                }
                break;
            case 7:
                // options (Map of authenticator options)
                ret = parse_options(&ctap2_request.options, &map, true);
                if (ret != CTAP1_ERR_SUCCESS) {
                    return ret;
                }
                break;
            case 8:
                // pinAuth（Byte Array）
                ret = parse_fixed_byte_string(&map, ctap2_request.pinAuth, PIN_AUTH_SIZE);
                if (ret != CTAP1_ERR_SUCCESS) {
                    // PINが正しく設定されていない旨のエラーを戻す
                    return CTAP2_ERR_PIN_NOT_SET;
                }
                break;
            case 9:
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

#if NRF_LOG_DEBUG_CLHASH_DATA_BUFF
    NRF_LOG_DEBUG("clientDataHash:");
    NRF_LOG_HEXDUMP_DEBUG(ctap2_request.clientDataHash, CLIENT_DATA_HASH_SIZE);
#endif

#if NRF_LOG_DEBUG_CBOR_REQUEST
    NRF_LOG_DEBUG("rp: id[%s] name[%s]", ctap2_request.rp.id, ctap2_request.rp.name);
    NRF_LOG_DEBUG("user: name[%s]", ctap2_request.user.name);
    NRF_LOG_DEBUG("user id(%d bytes):", ctap2_request.user.id_size);
    NRF_LOG_HEXDUMP_DEBUG(ctap2_request.user.id, ctap2_request.user.id_size);
    NRF_LOG_DEBUG("publicKeyCredentialTypeName: %s", 
        ctap2_request.cred_param.publicKeyCredentialTypeName);
    NRF_LOG_DEBUG("COSEAlgorithmIdentifier: %d", 
        ctap2_request.cred_param.COSEAlgorithmIdentifier);
    NRF_LOG_DEBUG("options: rk[%d] uv[%d] up[%d]", 
        ctap2_request.options.rk, ctap2_request.options.uv, ctap2_request.options.up);
    NRF_LOG_DEBUG("extensions: hmac-secret[%d]", ctap2_request.extensions.hmac_secret_requested);
#endif

#if NRF_LOG_DEBUG_PIN_AUTH
    NRF_LOG_DEBUG("pinAuth (pinProtocol=0x%02x):", ctap2_request.pinProtocol);
    NRF_LOG_HEXDUMP_DEBUG(ctap2_request.pinAuth, PIN_AUTH_SIZE);
#endif

    // 必須項目が揃っていない場合はエラー
    if (must_item_flag != 0x0f) {
        return CTAP2_ERR_MISSING_PARAMETER;
    }

    return CTAP1_ERR_SUCCESS;
}

bool ctap2_make_credential_is_tup_needed(void)
{
    return (ctap2_request.options.up == 1);
}

static uint8_t generate_credential_pubkey(void)
{
    // CBORエンコーダー初期化
    CborEncoder encoder;
    memset(credential_pubkey, 0x00, sizeof(credential_pubkey));
    cbor_encoder_init(&encoder, credential_pubkey, sizeof(credential_pubkey), 0);

    // CBORエンコード実行
    uint8_t *x = fido_crypto_keypair_public_key();
    uint8_t *y = fido_crypto_keypair_public_key() + 32;
    int32_t alg = ctap2_request.cred_param.COSEAlgorithmIdentifier;
    uint8_t ret = encode_cose_pubkey(&encoder, x, y, alg);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    // CBORエンコードデータ長を取得
    credential_pubkey_size = cbor_encoder_get_buffer_size(&encoder, credential_pubkey);

#if NRF_LOG_DEBUG_AUTH_DATA_ITEMS
    NRF_LOG_DEBUG("credentialPublicKey CBOR(%d bytes):", credential_pubkey_size);
    NRF_LOG_HEXDUMP_DEBUG(credential_pubkey, credential_pubkey_size);
#endif

    return CTAP1_ERR_SUCCESS;
}

static void generate_authenticator_data(void)
{
    // rpIdHashの先頭アドレスとサイズを取得
    uint8_t *ctap2_rpid_hash = ctap2_generated_rpid_hash();
    size_t   ctap2_rpid_hash_size = ctap2_generated_rpid_hash_size();

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
    //  attestedCredentialData
    //   aaguid
    uint8_t aaguid_size = ctap2_cbor_authgetinfo_aaguid_size();
    memcpy(authenticator_data + offset, ctap2_cbor_authgetinfo_aaguid(), aaguid_size);
    offset += aaguid_size;
    //   credentialIdLength
    uint8_t *credential_id = ctap2_pubkey_credential_id();
    size_t   credential_id_size = ctap2_pubkey_credential_id_size();
    fido_set_uint16_bytes(authenticator_data + offset, credential_id_size);
    offset += sizeof(uint16_t);
    //   credentialId
    memcpy(authenticator_data + offset, credential_id, credential_id_size);
    offset += credential_id_size;
    //   credentialPublicKey
    memcpy(authenticator_data + offset, credential_pubkey, credential_pubkey_size);
    offset += credential_pubkey_size;

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
    if (fido_flash_skey_cert_read() == false) {
        // 秘密鍵と証明書をFlash ROMから読込
        // NGであれば、エラーレスポンスを生成して戻す
        return CTAP2_ERR_VENDOR_FIRST;
    }

    if (fido_flash_skey_cert_available() == false) {
        // 秘密鍵と証明書がFlash ROMに登録されていない場合
        // エラーレスポンスを生成して戻す
        return CTAP2_ERR_VENDOR_FIRST;
    }

    if (ctap2_generate_signature(ctap2_request.clientDataHash, u2f_securekey_skey()) == false) {
        // 署名を実行
        // NGであれば、エラーレスポンスを生成して戻す
        return CTAP2_ERR_VENDOR_FIRST;
    }

#if NRF_LOG_DEBUG_SIGN_BUFF
    NRF_LOG_DEBUG("Signature(%d bytes):", u2f_crypto_signature_data_size());
    NRF_LOG_HEXDUMP_DEBUG(u2f_crypto_signature_data_buffer(), u2f_crypto_signature_data_size());
#endif

    return CTAP1_ERR_SUCCESS;
}

uint8_t ctap2_make_credential_generate_response_items(void)
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
    //   User Present result (0x01) &
    //   Attested credential data included (0x40)
    ctap2_flags_set(0x41);

    // sign counterをゼロクリア
    ctap2_set_sign_count(0);

    // Public Key Credential Sourceを編集する
    ctap2_pubkey_credential_generate_source(
        &ctap2_request.cred_param, &ctap2_request.user);

    // credentialIdを生成
    ctap2_pubkey_credential_generate_id();

    // credentialPublicKey(CBOR)を生成
    uint8_t ret = generate_credential_pubkey();
    if (ret != CTAP1_ERR_SUCCESS) {
        return ret;
    }

    // Authenticator dataを生成
    generate_authenticator_data();

    // 認証器に事前インストールされている
    // 秘密鍵を使用して署名を生成
    ret = generate_sign();
    if (ret != CTAP1_ERR_SUCCESS) {
        return ret;
    }
    
    return CTAP1_ERR_SUCCESS;
}

uint8_t ctap2_make_credential_encode_response(uint8_t *encoded_buff, size_t *encoded_buff_size)
{
    // CBORエンコーダーを初期化
    CborEncoder encoder;
    cbor_encoder_init(&encoder, encoded_buff, *encoded_buff_size, 0);

    // Map初期化
    CborEncoder map;
    CborError ret = cbor_encoder_create_map(&encoder, &map, 3);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    // fmt (0x01: RESP_fmt)
    ret = cbor_encode_int(&map, 0x01);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = cbor_encode_text_stringz(&map, "packed");
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
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

    // attStmt (0x03: RESP_attStmt)
    ret = cbor_encode_int(&map, 0x03);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    // 証明書は入れ子のCBORとなる
    CborEncoder stmtmap;
    CborEncoder x5carr;
    ret = cbor_encoder_create_map(&map, &stmtmap, 3);
    if (ret == CborNoError) {
        // alg
        ret = cbor_encode_text_stringz(&stmtmap,"alg");
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        ret = cbor_encode_int(&stmtmap,COSE_ALG_ES256);
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        // sig
        ret = cbor_encode_text_stringz(&stmtmap,"sig");
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        ret = cbor_encode_byte_string(&stmtmap,
            u2f_crypto_signature_data_buffer(), u2f_crypto_signature_data_size());
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        // x5c
        ret = cbor_encode_text_stringz(&stmtmap,"x5c");
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        ret = cbor_encoder_create_array(&stmtmap, &x5carr, 1);
        if (ret == CborNoError) {
            // 証明書格納領域と長さを取得
            uint8_t *cert_buffer = u2f_securekey_cert();
            uint32_t cert_buffer_length = u2f_securekey_cert_length();
            // 証明書を格納
            ret = cbor_encode_byte_string(&x5carr, cert_buffer, cert_buffer_length);
            if (ret != CborNoError) {
                return CTAP1_ERR_OTHER;
            }
            ret = cbor_encoder_close_container(&stmtmap, &x5carr);
            if (ret != CborNoError) {
                return CTAP1_ERR_OTHER;
            }
        }
    }

    ret = cbor_encoder_close_container(&map, &stmtmap);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    ret = cbor_encoder_close_container(&encoder, &map);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    // CBORバッファの長さを設定
    *encoded_buff_size = cbor_encoder_get_buffer_size(&encoder, encoded_buff);

#if NRF_LOG_DEBUG_CBOR_RESPONSE
    NRF_LOG_DEBUG("authenticatorMakeCredential response(%d bytes):", *encoded_buff_size);
    int j, k;
    int max = 288;
    for (j = 0; j < max; j += 64) {
        k = max - j;
        NRF_LOG_HEXDUMP_DEBUG(encoded_buff + j, (k < 64) ? k : 64);
    }
#endif

    return CTAP1_ERR_SUCCESS;
}

uint8_t ctap2_make_credential_add_token_counter(void)
{
    // 例外抑止
    if (ctap2_pubkey_credential_source_hash_size() != sizeof(nrf_crypto_hash_sha256_digest_t)) {
        return CTAP1_ERR_OTHER;
    }

    // Public Key Credential Sourceから
    // 生成されたSHA-256ハッシュ値をキーとし、
    // トークンカウンターレコードを追加する
    uint8_t *p_hash = ctap2_pubkey_credential_source_hash();
    uint8_t *p_hash_for_check = ctap2_generated_rpid_hash();
    if (fido_flash_token_counter_write(p_hash, ctap2_current_sign_count(), p_hash_for_check) == false) {
        return CTAP1_ERR_OTHER;
    }

    // 後続のレスポンス生成・送信は、
    // Flash ROM書込み完了後に行われる
    NRF_LOG_DEBUG("sign counter registered (value=%d)", ctap2_current_sign_count());
    return CTAP1_ERR_SUCCESS;
}

uint8_t ctap2_make_credential_verify_pin_auth(void)
{
    // PIN認証が必要でない場合は終了
    if (ctap2_request.pinProtocol != 0x01) {
        return CTAP1_ERR_SUCCESS;
    }

    // pinAuthの妥当性チェックを行い、
    // NGの場合はPIN認証失敗
    uint8_t ctap2_status = ctap2_client_pin_token_verify_pin_auth(ctap2_request.clientDataHash, ctap2_request.pinAuth);
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        NRF_LOG_ERROR("pinAuth verification failed");
        return ctap2_status;
    }

    // flagsを設定
    //   User Verified result (0x04)
    ctap2_flags_set(0x04);
    NRF_LOG_DEBUG("pinAuth verification success");
    return CTAP1_ERR_SUCCESS;
}
