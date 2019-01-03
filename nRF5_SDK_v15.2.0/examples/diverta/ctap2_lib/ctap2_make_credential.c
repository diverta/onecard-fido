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
#include "fido_common.h"
#include "fido_crypto.h"
#include "fido_crypto_ecb.h"
#include "fido_crypto_keypair.h"

// for u2f_flash_keydata_read & u2f_flash_keydata_available
#include "u2f_flash.h"

// for u2f_crypto_sign & other
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

// credentialPublicKeyを保持
static uint8_t credential_pubkey[80];
static size_t  credential_pubkey_size;

// Authenticator dataを保持
static uint8_t authenticator_data[256];
static size_t  authenticator_data_size;

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

#if NRF_LOG_DEBUG_CLHASH_DATA_BUFF
    NRF_LOG_DEBUG("clientDataHash:");
    NRF_LOG_HEXDUMP_DEBUG(make_credential_request.clientDataHash, CLIENT_DATA_HASH_SIZE);
#endif

#if NRF_LOG_DEBUG_CBOR_REQUEST
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

static void generate_rpid_hash(void)
{
    // RP IDからSHA-256ハッシュ（32バイト）を生成 
    uint8_t *rpid = make_credential_request.rp.id;
    size_t   rpid_size = strlen((char *)rpid);
    ctap2_rpid_hash_size = sizeof(ctap2_rpid_hash);
    fido_crypto_generate_sha256_hash(rpid, rpid_size, ctap2_rpid_hash, &ctap2_rpid_hash_size);

#if NRF_LOG_DEBUG_AUTH_DATA_ITEMS
    NRF_LOG_DEBUG("RP ID[%s](%d bytes) hash value:", rpid, rpid_size);
    NRF_LOG_HEXDUMP_DEBUG(ctap2_rpid_hash, ctap2_rpid_hash_size);
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

#if NRF_LOG_DEBUG_AUTH_DATA_ITEMS
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

#if NRF_LOG_DEBUG_AUTH_DATA_ITEMS
    NRF_LOG_DEBUG("credentialId(%d bytes):", credential_id_size);
    NRF_LOG_HEXDUMP_DEBUG(credential_id, credential_id_size);
#endif
}

static uint8_t encode_credential_pubkey(CborEncoder *encoder, uint8_t *x, uint8_t *y, int32_t alg)
{
    uint8_t ret;
    CborEncoder map;

    ret = cbor_encoder_create_map(encoder, &map, 5);
    if (ret != CborNoError) {
        return ret;
    }

    // Key type
    ret = cbor_encode_int(&map, COSE_KEY_LABEL_KTY);
    if (ret != CborNoError) {
        return ret;
    }
    ret = cbor_encode_int(&map, COSE_KEY_KTY_EC2);
    if (ret != CborNoError) {
        return ret;
    }

    // Signature algorithm
    ret = cbor_encode_int(&map, COSE_KEY_LABEL_ALG);
    if (ret != CborNoError) {
        return ret;
    }
    ret = cbor_encode_int(&map, alg);
    if (ret != CborNoError) {
        return ret;
    }

    // Curve type
    ret = cbor_encode_int(&map, COSE_KEY_LABEL_CRV);
    if (ret != CborNoError) {
        return ret;
    }
    ret = cbor_encode_int(&map, COSE_KEY_CRV_P256);
    if (ret != CborNoError) {
        return ret;
    }

    // x-coordinate
    ret = cbor_encode_int(&map, COSE_KEY_LABEL_X);
    if (ret != CborNoError) {
        return ret;
    }
    ret = cbor_encode_byte_string(&map, x, 32);
    if (ret != CborNoError) {
        return ret;
    }

    // y-coordinate
    ret = cbor_encode_int(&map, COSE_KEY_LABEL_Y);
    if (ret != CborNoError) {
        return ret;
    }
    ret = cbor_encode_byte_string(&map, y, 32);
    if (ret != CborNoError) {
        return ret;
    }

    ret = cbor_encoder_close_container(encoder, &map);
    if (ret != CborNoError) {
        return ret;
    }

    return CborNoError;
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
    int32_t alg = make_credential_request.cred_param.COSEAlgorithmIdentifier;
    uint8_t ret = encode_credential_pubkey(&encoder, x, y, alg);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }

    // CBORエンコードデータ長を取得
    credential_pubkey_size = cbor_encoder_get_buffer_size(&encoder, credential_pubkey);

#if NRF_LOG_DEBUG_AUTH_DATA_ITEMS
    NRF_LOG_DEBUG("credentialPublicKey CBOR(%d bytes):", credential_pubkey_size);
    NRF_LOG_HEXDUMP_DEBUG(credential_pubkey, credential_pubkey_size);
#endif

    return CTAP1_ERR_SUCCESS;
}

void generate_authenticator_data(void)
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
    //  attestedCredentialData
    //   aaguid
    int aaguid_size = ctap2_cbor_authgetinfo_aaguid_size();
    memcpy(authenticator_data + offset, ctap2_cbor_authgetinfo_aaguid(), aaguid_size);
    offset += aaguid_size;
    //   credentialIdLength
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

uint8_t generate_sign(void)
{
    if (u2f_flash_keydata_read() == false) {
        // 秘密鍵と証明書をFlash ROMから読込
        // NGであれば、エラーレスポンスを生成して戻す
        return CTAP2_ERR_VENDOR_FIRST;
    }

    if (u2f_flash_keydata_available() == false) {
        // 秘密鍵と証明書がFlash ROMに登録されていない場合
        // エラーレスポンスを生成して戻す
        return CTAP2_ERR_VENDOR_FIRST;
    }

    // 署名生成用バッファの格納領域を取得
    uint8_t offset = 0;
    uint8_t *signature_base_buffer = u2f_crypto_signature_data_buffer();

    // Authenticator data
    memcpy(signature_base_buffer + offset, authenticator_data, authenticator_data_size);
    offset += authenticator_data_size;

    // clientDataHash 
    memcpy(signature_base_buffer + offset, make_credential_request.clientDataHash, CLIENT_DATA_HASH_SIZE);
    offset += CLIENT_DATA_HASH_SIZE;

    // メッセージのバイト数をセット
    u2f_crypto_signature_data_size_set(offset);

    // 署名用の秘密鍵を取得し、署名を生成
    if (u2f_crypto_sign(u2f_securekey_skey_be()) != NRF_SUCCESS) {
        // 署名生成に失敗したら終了
        return false;
    }

    // ASN.1形式署名を格納する領域を準備
    if (u2f_crypto_create_asn1_signature() == false) {
        // 生成された署名をASN.1形式署名に変換する
        // 変換失敗の場合終了
        return false;
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
    generate_rpid_hash();
    
    // flags編集
    //   User Present result (0x01) &
    //   Attested credential data included (0x40)
    ctap2_flags = 0x41;

    // credentialIdを生成
    generate_credential_id();

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
    int         ret;
    ret = cbor_encoder_create_map(&encoder, &map, 3);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }

    // fmt (0x01: RESP_fmt)
    ret = cbor_encode_int(&map, 0x01);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }
    ret = cbor_encode_text_stringz(&map, "packed");
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
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

    // attStmt (0x03: RESP_attStmt)
    ret = cbor_encode_int(&map, 0x03);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }

    // 証明書は入れ子のCBORとなる
    CborEncoder stmtmap;
    CborEncoder x5carr;
    ret = cbor_encoder_create_map(&map, &stmtmap, 3);
    if (ret == CborNoError) {
        // alg
        ret = cbor_encode_text_stringz(&stmtmap,"alg");
        if (ret != CborNoError) {
            return CTAP2_ERR_PROCESSING;
        }
        ret = cbor_encode_int(&stmtmap,COSE_ALG_ES256);
        if (ret != CborNoError) {
            return CTAP2_ERR_PROCESSING;
        }
        // sig
        ret = cbor_encode_text_stringz(&stmtmap,"sig");
        if (ret != CborNoError) {
            return CTAP2_ERR_PROCESSING;
        }
        ret = cbor_encode_byte_string(&stmtmap,
            u2f_crypto_signature_data_buffer(), u2f_crypto_signature_data_size());
        if (ret != CborNoError) {
            return CTAP2_ERR_PROCESSING;
        }
        // x5c
        ret = cbor_encode_text_stringz(&stmtmap,"x5c");
        if (ret != CborNoError) {
            return CTAP2_ERR_PROCESSING;
        }
        ret = cbor_encoder_create_array(&stmtmap, &x5carr, 1);
        if (ret == CborNoError) {
            // 証明書格納領域と長さを取得
            uint8_t *cert_buffer = u2f_securekey_cert();
            uint32_t cert_buffer_length = u2f_securekey_cert_length();
            // 証明書を格納
            ret = cbor_encode_byte_string(&x5carr, cert_buffer, cert_buffer_length);
            if (ret != CborNoError) {
                return CTAP2_ERR_PROCESSING;
            }
            ret = cbor_encoder_close_container(&stmtmap, &x5carr);
            if (ret != CborNoError) {
                return CTAP2_ERR_PROCESSING;
            }
        }
    }

    ret = cbor_encoder_close_container(&map, &stmtmap);
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
    if (ctap2_rpid_hash_size != sizeof(nrf_crypto_hash_sha256_digest_t)) {
        return CTAP2_ERR_PROCESSING;
    }
    
    // rpIdHashをキーとして、
    // トークンカウンターレコードを追加する
    uint32_t reserve_word = 0xffffffff;
    if (u2f_flash_token_counter_write(ctap2_rpid_hash, ctap2_sign_count, reserve_word) == false) {
        return CTAP2_ERR_PROCESSING;
    }

    // 後続のレスポンス生成・送信は、
    // Flash ROM書込み完了後に行われる
    NRF_LOG_DEBUG("sign counter registered (value=%d)", ctap2_sign_count);
    return CTAP1_ERR_SUCCESS;
}
