/* 
 * File:   ctap2_client_pin.c
 * Author: makmorit
 *
 * Created on 2019/02/18, 11:05
 */
#include "sdk_common.h"

#include "cbor.h"
#include "ctap2_common.h"
#include "ctap2_cbor_authgetinfo.h"
#include "ctap2_cbor_parse.h"
#include "ctap2_key_agreement.h"
#include "ctap2_pubkey_credential.h"
#include "ctap2_client_pin_sskey.h"
#include "ctap2_client_pin_token.h"
#include "fido_common.h"
#include "fido_crypto_keypair.h"

// for u2f_flash_keydata_read & u2f_flash_keydata_available
#include "fido_flash.h"

// for u2f_crypto_signature_data
#include "u2f_crypto.h"

// for u2f_securekey_skey_be
#include "u2f_register.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ctap2_client_pin
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for debug cbor data
#define NRF_LOG_HEXDUMP_DEBUG_CBOR      false
#define NRF_LOG_DEBUG_CBOR_REQUEST      false
#define NRF_LOG_DEBUG_CALCULATED_HMAC   false

// デコードされた
// authenticatorClientPIN
// リクエストデータを保持する構造体
#define PIN_AUTH_SIZE           16
#define NEW_PIN_ENC_MAX_SIZE    256
#define NEW_PIN_ENC_MIN_SIZE    64
#define PIN_HASH_ENC_SIZE       16
struct {
    uint8_t pinProtocol;
    uint8_t subCommand;
    CTAP_COSE_KEY cose_key;
    uint8_t pinAuth[PIN_AUTH_SIZE];
    size_t  pinAuthSize;
    uint8_t newPinEnc[NEW_PIN_ENC_MAX_SIZE];
    size_t  newPinEncSize;
    uint8_t pinHashEnc[PIN_HASH_ENC_SIZE];
    size_t  pinHashEncSize;
} ctap2_request;

// サブコマンド定義
#define subcmd_GetRetries       0x01
#define subcmd_GetKeyAgreement  0x02
#define subcmd_SetPin           0x03
#define subcmd_ChangePin        0x04
#define subcmd_GetPinToken      0x05


static void debug_decoded_request()
{
#if NRF_LOG_DEBUG_CBOR_REQUEST
    NRF_LOG_DEBUG("pinProtocol(0x%02x) subCommand(0x%02x)", ctap2_request.pinProtocol, ctap2_request.subCommand);

    NRF_LOG_DEBUG("keyAgreement: alg(%d) curve(%d) public key(64 bytes):",
        ctap2_request.cose_key.alg, ctap2_request.cose_key.crv);
    NRF_LOG_HEXDUMP_DEBUG(ctap2_request.cose_key.key.x, 32);
    NRF_LOG_HEXDUMP_DEBUG(ctap2_request.cose_key.key.y, 32);

    NRF_LOG_DEBUG("pinAuth(%dbytes):", PIN_AUTH_SIZE);
    NRF_LOG_HEXDUMP_DEBUG(ctap2_request.pinAuth, PIN_AUTH_SIZE);

    NRF_LOG_DEBUG("newPinEnc(%dbytes):", ctap2_request.newPinEncSize);
    int j, k;
    int max = (ctap2_request.newPinEncSize < NEW_PIN_ENC_MAX_SIZE) ? ctap2_request.newPinEncSize : NEW_PIN_ENC_MAX_SIZE;
    for (j = 0; j < max; j += 64) {
        k = max - j;
        NRF_LOG_HEXDUMP_DEBUG(ctap2_request.newPinEnc + j, (k < 64) ? k : 64);
    }

    NRF_LOG_DEBUG("pinHashEnc(%dbytes):", PIN_HASH_ENC_SIZE);
    NRF_LOG_HEXDUMP_DEBUG(ctap2_request.pinHashEnc, PIN_HASH_ENC_SIZE);
#endif
}

uint8_t ctap2_client_pin_decode_request(uint8_t *cbor_data_buffer, size_t cbor_data_length)
{
    CborParser  parser;
    CborValue   it;
    CborValue   map;
    size_t      map_length;
    CborType    type;
    CborError   ret;
    uint8_t     i;
    int         key;
    size_t      sz;

#if NRF_LOG_HEXDUMP_DEBUG_CBOR
    NRF_LOG_DEBUG("authenticatorClientPIN request cbor(%d bytes):", cbor_data_length);
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
                // pinProtocol (Unsigned Integer)
                if (cbor_value_get_type(&map) != CborIntegerType) {
                    return CTAP2_ERR_INVALID_CBOR_TYPE;
                }
                if (cbor_value_get_int_checked(&map, (int *)&ctap2_request.pinProtocol) != CborNoError) {
                    return CTAP2_ERR_CBOR_PARSING;
                }
                must_item_flag |= 0x01;
                break;
            case 2:
                // subCommand (Unsigned Integer)
                if (cbor_value_get_type(&map) != CborIntegerType) {
                    return CTAP2_ERR_INVALID_CBOR_TYPE;
                }
                if (cbor_value_get_int_checked(&map, (int *)&ctap2_request.subCommand) != CborNoError) {
                    return CTAP2_ERR_CBOR_PARSING;
                }
                must_item_flag |= 0x02;
                break;
            case 3:
                // keyAgreement (COSE_Key)
                ret = parse_cose_pubkey(&map, &ctap2_request.cose_key);
                if (ret != CTAP1_ERR_SUCCESS) {
                    return ret;
                }
                break;
            case 4:
                // pinAuth (Byte Array)
                ret = parse_fixed_byte_string(&map, ctap2_request.pinAuth, PIN_AUTH_SIZE);
                if (ret != CTAP1_ERR_SUCCESS) {
                    return ret;
                }
                ctap2_request.pinAuthSize = PIN_AUTH_SIZE;
                break;
            case 5:
                // newPinEnc (Byte Array)
                if (cbor_value_get_type(&map) != CborByteStringType) {
                    return CTAP2_ERR_INVALID_CBOR_TYPE;
                }
                if (cbor_value_calculate_string_length(&map, &sz) != CborNoError) {
                    return CTAP2_ERR_CBOR_PARSING;
                }
                if (sz > NEW_PIN_ENC_MAX_SIZE || sz < NEW_PIN_ENC_MIN_SIZE) {
                    return CTAP2_ERR_PIN_POLICY_VIOLATION;
                }
                ctap2_request.newPinEncSize = sz;
                sz = NEW_PIN_ENC_MAX_SIZE;
                if (cbor_value_copy_byte_string(&map, ctap2_request.newPinEnc, &sz, NULL) != CborNoError) {
                    return CTAP2_ERR_CBOR_PARSING;
                }
                break;
            case 6:
                // pinHashEnc (Byte Array)
                ret = parse_fixed_byte_string(&map, ctap2_request.pinHashEnc, PIN_HASH_ENC_SIZE);
                if (ret != CTAP1_ERR_SUCCESS) {
                    return ret;
                }
                ctap2_request.pinHashEncSize = PIN_HASH_ENC_SIZE;
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

uint8_t encode_get_key_agreement_response(uint8_t *encoded_buff, size_t *encoded_buff_size)
{
    // 鍵交換用キーペアが未生成の場合は新規生成
    // (再生成は要求しない)
    ctap2_client_pin_sskey_init(false);

    // レスポンスをエンコード
    uint8_t ret = ctap2_key_agreement_encode_response(encoded_buff, encoded_buff_size);
    if (ret == CTAP1_ERR_SUCCESS) {
        return ret;
    }
    
    NRF_LOG_DEBUG("getKeyAgreement: public key generate success");
    return CTAP1_ERR_SUCCESS;
}

uint8_t verify_pin_auth(void)
{
    // 共通鍵ハッシュを利用し、
    // CTAP2クライアントから受領したPINデータを
    // HMAC SHA-256アルゴリズムでハッシュ化
    uint8_t *hmac = ctap2_client_pin_sskey_calculate_hmac(
        ctap2_request.newPinEnc, ctap2_request.newPinEncSize,
        ctap2_request.pinHashEnc, ctap2_request.pinHashEncSize);
    if (hmac == NULL) {
        return CTAP2_ERR_PROCESSING;
    }

#if NRF_LOG_DEBUG_CALCULATED_HMAC
    NRF_LOG_DEBUG("Calculated hmac(%dbytes):", ctap2_request.pinAuthSize);
    NRF_LOG_HEXDUMP_DEBUG(hmac, ctap2_request.pinAuthSize);
    NRF_LOG_DEBUG("pinAuth(%dbytes):", ctap2_request.pinAuthSize);
    NRF_LOG_HEXDUMP_DEBUG(ctap2_request.pinAuth, ctap2_request.pinAuthSize);
#endif

    // クライアントから受信したpinAuth（16バイト）を、
    // PINデータから生成されたHMAC SHA-256ハッシュと比較し、
    // 異なる場合はエラーを戻す
    if (memcmp(hmac, ctap2_request.pinAuth, ctap2_request.pinAuthSize) != 0) {
        return CTAP2_ERR_PIN_AUTH_INVALID;
    }

    NRF_LOG_DEBUG("setPIN: pinAuth verify success");
    return CTAP1_ERR_SUCCESS;
}

uint8_t encode_set_pin_response(uint8_t *encoded_buff, size_t *encoded_buff_size)
{
    // CTAP2クライアントから受け取った公開鍵と、
    // 鍵交換用キーペアの秘密鍵を使用し、共通鍵ハッシュを生成
    uint8_t ret = ctap2_client_pin_sskey_generate((uint8_t *)&ctap2_request.cose_key.key);
    if (ret != CTAP1_ERR_SUCCESS) {
        // 鍵交換用キーペアが未生成の場合はエラー
        return ret;
    }

    // CTAP2クライアントから受け取ったHMACハッシュを、
    // 共通鍵ハッシュを使用して検証
    ret = verify_pin_auth();
    if (ret != CTAP1_ERR_SUCCESS) {
        return ret;
    }

    return CTAP1_ERR_SUCCESS;
}

uint8_t ctap2_client_pin_encode_response(uint8_t *encoded_buff, size_t *encoded_buff_size)
{
    NRF_LOG_DEBUG("authenticatorClientPIN start: subcommand(0x%02x)", ctap2_request.subCommand);

    uint8_t ret = CTAP1_ERR_OTHER;
    switch (ctap2_request.subCommand) {
        case subcmd_GetKeyAgreement:
            ret = encode_get_key_agreement_response(encoded_buff, encoded_buff_size);
            break;
        case subcmd_SetPin:
            ret = encode_set_pin_response(encoded_buff, encoded_buff_size);
            break;
        default:
            break;
    }

    return ret;
}
