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
#define NRF_LOG_HEXDUMP_DEBUG_CBOR      true
#define NRF_LOG_DEBUG_CBOR_REQUEST      true
#define NRF_LOG_DEBUG_AUTH_DATA_ITEMS   false
#define NRF_LOG_DEBUG_AUTH_DATA_BUFF    false
#define NRF_LOG_DEBUG_SIGN_BUFF         false
#define NRF_LOG_DEBUG_CBOR_RESPONSE     false


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
    uint8_t newPinEnc[NEW_PIN_ENC_MAX_SIZE];
    size_t  newPinEncSize;
    uint8_t pinHashEnc[PIN_HASH_ENC_SIZE];
} ctap2_request;

// サブコマンド定義
#define subcmd_GetRetries       0x01
#define subcmd_GetKeyAgreement  0x02
#define subcmd_SetPin           0x03
#define subcmd_ChangePin        0x04
#define subcmd_GetPinToken      0x05

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
                break;
            default:
                break;
        }

        ret = cbor_value_advance(&map);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }
    }
#if NRF_LOG_DEBUG_CBOR_REQUEST
    NRF_LOG_DEBUG("pinProtocol(0x%02x) subCommand(0x%02x)", ctap2_request.pinProtocol, ctap2_request.subCommand);
#endif

    // 必須項目が揃っていない場合はエラー
    if (must_item_flag != 0x03) {
        return CTAP2_ERR_MISSING_PARAMETER;
    }

    return CTAP1_ERR_SUCCESS;
}

uint8_t encode_get_key_agreement_response(uint8_t *encoded_buff, size_t *encoded_buff_size)
{
    // キーペアを生成
    ctap2_key_agreement_generate_keypair();

    // レスポンスをエンコード
    uint8_t ret = ctap2_key_agreement_encode_response(encoded_buff, encoded_buff_size);
    if (ret == CTAP1_ERR_SUCCESS) {
        return ret;
    }
    
    return CTAP1_ERR_SUCCESS;
}


uint8_t encode_set_pin_response(uint8_t *encoded_buff, size_t *encoded_buff_size)
{
    return CTAP1_ERR_OTHER;
}

uint8_t ctap2_client_pin_encode_response(uint8_t *encoded_buff, size_t *encoded_buff_size)
{
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
