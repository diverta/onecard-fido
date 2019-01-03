/* 
 * File:   ctap2_get_assertion.c
 * Author: makmorit
 *
 * Created on 2019/01/03, 11:05
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
#define NRF_LOG_MODULE_NAME ctap2_get_assertion
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for debug cbor data
#define NRF_LOG_DEBUG_CLHASH_DATA_BUFF  true
#define NRF_LOG_HEXDUMP_DEBUG_CBOR      true
#define NRF_LOG_DEBUG_CBOR_REQUEST      true
#define NRF_LOG_DEBUG_AUTH_DATA_ITEMS   false
#define NRF_LOG_DEBUG_AUTH_DATA_BUFF    false
#define NRF_LOG_DEBUG_SIGN_BUFF         false
#define NRF_LOG_DEBUG_CBOR_RESPONSE     false

// デコードされた
// authenticatorGetAssertion
// リクエストデータを保持する構造体
struct {
    uint32_t                 paramsParsed;
    bool                     clientDataHashPresent;
    uint8_t                  clientDataHash[CLIENT_DATA_HASH_SIZE];
    CTAP_RP_ID_T             rp;
    CTAP_USER_ENTITY_T       user;
    CTAP_PUBKEY_CRED_PARAM_T cred_param;
    CTAP_OPTIONS_T           options;
} ctap2_request;

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
                break;
            case 5:
                // options (Map of authenticator options)
                ret = parse_options(&ctap2_request.options ,&map);
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
    NRF_LOG_HEXDUMP_DEBUG(ctap2_request.clientDataHash, CLIENT_DATA_HASH_SIZE);
#endif

#if NRF_LOG_DEBUG_CBOR_REQUEST
    NRF_LOG_DEBUG("rp id[%s]", ctap2_request.rp.id);
    //NRF_LOG_DEBUG("user: id[%s] name[%s]", make_credential_request.user.id, make_credential_request.user.name);
    //NRF_LOG_DEBUG("publicKeyCredentialTypeName: %s", 
    //    make_credential_request.cred_param.publicKeyCredentialTypeName);
    //NRF_LOG_DEBUG("COSEAlgorithmIdentifier: %d", 
    //    make_credential_request.cred_param.COSEAlgorithmIdentifier);
    NRF_LOG_DEBUG("options: rk[%d] uv[%d] up[%d]", 
        ctap2_request.options.rk, ctap2_request.options.uv, ctap2_request.options.up);
#endif

    return CTAP1_ERR_SUCCESS;
}

bool ctap2_get_assertion_is_tup_needed(void)
{
    return true;
}

uint8_t ctap2_get_assertion_generate_response_items(void)
{
    // TODO: 仮の実装です。
    return CTAP2_ERR_PROCESSING;
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
