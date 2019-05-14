/* 
 * File:   ctap_cbor_parse.c
 * Author: makmorit
 *
 * Created on 2018/12/25, 13:21
 */
#include "sdk_common.h"

#include "cbor.h"
#include "ctap2_common.h"
#include "ctap2_cbor_parse.h"
#include "ctap2_pubkey_credential.h"
#include "fido_common.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ctap_cbor_parse
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for debug
#define NRF_LOG_DEBUG_CRED_DESC false

uint8_t parse_fixed_byte_string(CborValue *map, uint8_t *dst, int len)
{
    if (cbor_value_get_type(map) != CborByteStringType) {
        return CTAP2_ERR_INVALID_CBOR_TYPE;
    }

    size_t sz = len;
    int ret = cbor_value_copy_byte_string(map, dst, &sz, NULL);
    if (ret != CborNoError) {
        return CTAP2_ERR_CBOR_PARSING;
    }
    if (sz != len) {
        return CTAP1_ERR_OTHER;
    }

    return CTAP1_ERR_SUCCESS;
}

uint8_t parse_rp_id(CTAP_RP_ID_T* rp, CborValue *val)
{
    if (cbor_value_get_type(val) != CborTextStringType) {
        return CTAP2_ERR_INVALID_CBOR_TYPE;
    }

    size_t sz = RP_ID_MAX_SIZE;
    CborError ret = cbor_value_copy_text_string(val, (char*)rp->id, &sz, NULL);
    if (ret == CborErrorOutOfMemory) {
        return CTAP2_ERR_LIMIT_EXCEEDED;
    }
    if (ret != CborNoError) {
        return CTAP2_ERR_CBOR_PARSING;
    }

    rp->id[RP_ID_MAX_SIZE] = 0;
    rp->id_size = sz;

    return CTAP1_ERR_SUCCESS;
}

uint8_t parse_rp(CTAP_RP_ID_T *rp, CborValue *val)
{
    size_t    sz;
    size_t    map_length;
    char      key[16];
    CborError ret;
    uint8_t   i;
    CborValue map;

    if (cbor_value_get_type(val) != CborMapType) {
        return CTAP2_ERR_INVALID_CBOR;
    }

    ret = cbor_value_enter_container(val,&map);
    if (ret != CborNoError) {
        return ret;
    }

    ret = cbor_value_get_map_length(val, &map_length);
    if (ret != CborNoError) {
        return ret;
    }

    rp->id_size = 0;
    for (i = 0; i < map_length; i++) {
        if (cbor_value_get_type(&map) != CborTextStringType) {
            return CTAP2_ERR_INVALID_CBOR;
        }

        sz = sizeof(key);
        ret = cbor_value_copy_text_string(&map, key, &sz, NULL);
        if (ret == CborErrorOutOfMemory) {
            return CTAP2_ERR_LIMIT_EXCEEDED;
        }
        if (ret != CborNoError) {
            return ret;
        }
        key[sizeof(key) - 1] = 0;

        ret = cbor_value_advance(&map);
        if (ret != CborNoError) {
            return ret;
        }

        if (cbor_value_get_type(&map) != CborTextStringType) {
            return CTAP2_ERR_INVALID_CBOR_TYPE;
        }

        if (strcmp(key, "id") == 0) {
            ret = parse_rp_id(rp, &map);
            if (ret != CTAP1_ERR_SUCCESS) {
                return ret;
            }

        } else if (strcmp(key, "name") == 0) {
            sz = RP_NAME_MAX_SIZE;
            ret = cbor_value_copy_text_string(&map, (char*)rp->name, &sz, NULL);
            if (ret != CborErrorOutOfMemory && ret != CborNoError) {
                // Just truncate the name it's okay
                return ret;
            }
            rp->name[RP_NAME_MAX_SIZE - 1] = 0;
        }

        ret = cbor_value_advance(&map);
        if (ret != CborNoError) {
            return ret;
        }

    }

    if (rp->id_size == 0) {
        return CTAP2_ERR_MISSING_PARAMETER;
    }

    return CborNoError;
}

uint8_t parse_user(CTAP_USER_ENTITY_T *user, CborValue *val)
{
    size_t    sz;
    size_t    map_length;
    uint8_t   key[16];
    CborError ret;
    uint8_t   i;
    CborValue map;

    if (cbor_value_get_type(val) != CborMapType) {
        return CTAP2_ERR_INVALID_CBOR_TYPE;
    }

    ret = cbor_value_enter_container(val,&map);
    if (ret != CborNoError) {
        return ret;
    }

    ret = cbor_value_get_map_length(val, &map_length);
    if (ret != CborNoError) {
        return ret;
    }

    for (i = 0; i < map_length; i++) {
        if (cbor_value_get_type(&map) != CborTextStringType) {
            return CTAP2_ERR_INVALID_CBOR_TYPE;
        }

        sz = sizeof(key);
        ret = cbor_value_copy_text_string(&map, (char *)key, &sz, NULL);
        if (ret == CborErrorOutOfMemory) {
            return CTAP2_ERR_LIMIT_EXCEEDED;
        }
        if (ret != CborNoError) {
            return ret;
        }
        key[sizeof(key) - 1] = 0;

        ret = cbor_value_advance(&map);
        if (ret != CborNoError) {
            return ret;
        }

        if (strcmp((const char*)key, "id") == 0) {
            if (cbor_value_get_type(&map) != CborByteStringType) {
                return CTAP2_ERR_INVALID_CBOR_TYPE;
            }

            sz = USER_ID_MAX_SIZE;
            ret = cbor_value_copy_byte_string(&map, user->id, &sz, NULL);
            if (ret == CborErrorOutOfMemory) {
                return CTAP2_ERR_LIMIT_EXCEEDED;
            }
            user->id_size = sz;
            if (ret != CborNoError) {
                return ret;
            }

        } else if (strcmp((const char *)key, "name") == 0) {
            if (cbor_value_get_type(&map) != CborTextStringType) {
                return CTAP2_ERR_INVALID_CBOR_TYPE;
            }

            sz = USER_NAME_MAX_SIZE;
            ret = cbor_value_copy_text_string(&map, (char *)user->name, &sz, NULL);
            if (ret != CborErrorOutOfMemory && ret != CborNoError) {
                // Just truncate the name it's okay
                return ret;
            }
            user->name[USER_NAME_MAX_SIZE - 1] = 0;

        } else if (strcmp((const char *)key, "displayName") == 0) {
            // ユーザー名表示はサポートしないので
            // 項目の型チェックのみを行う
            if (cbor_value_get_type(&map) != CborTextStringType) {
                return CTAP2_ERR_INVALID_CBOR_TYPE;
            }

        } else if (strcmp((const char *)key, "icon") == 0) {
            // ユーザーアイコンはサポートしないので
            // 項目の型チェックのみを行う
            if (cbor_value_get_type(&map) != CborTextStringType) {
                return CTAP2_ERR_INVALID_CBOR_TYPE;
            }
        }

        ret = cbor_value_advance(&map);
        if (ret != CborNoError) {
            return ret;
        }
    }

    return CborNoError;
}

uint8_t check_pub_key_cred_param(CborValue *val)
{
    CborValue cred;
    CborValue alg;
    CborError ret;

    if (cbor_value_get_type(val) != CborMapType) {
        return CTAP2_ERR_INVALID_CBOR_TYPE;
    }

    ret = cbor_value_map_find_value(val, "type", &cred);
    if (ret != CborNoError) {
        return ret;
    }
    if (cbor_value_get_type(&cred) != CborTextStringType) {
        return CTAP2_ERR_MISSING_PARAMETER;
    }

    ret = cbor_value_map_find_value(val, "alg", &alg);
    if (ret != CborNoError) {
        return ret;
    }
    if (cbor_value_get_type(&alg) != CborIntegerType) {
        return CTAP2_ERR_MISSING_PARAMETER;
    }

    return CborNoError;
}

uint8_t parse_pub_key_cred_param(CborValue *val, CTAP_PUBKEY_CRED_PARAM_T *cred_param)
{
    CborValue cred;
    CborValue alg;
    CborError ret;
    char      type_str[16];
    size_t    sz = sizeof(type_str);

    // type の内容を取得
    ret = cbor_value_map_find_value(val, "type", &cred);
    if (ret != CborNoError) {
        return ret;
    }
    ret = cbor_value_copy_text_string(&cred, type_str, &sz, NULL);
    if (ret != CborNoError) {
        return ret;
    }
    type_str[sizeof(type_str) - 1] = 0;
    if (strcmp(type_str, "public-key") == 0) {
        strcpy((char *)cred_param->publicKeyCredentialTypeName, type_str);
        cred_param->publicKeyCredentialType = PUB_KEY_CRED_PUB_KEY;
    } else {
        cred_param->publicKeyCredentialType = PUB_KEY_CRED_UNKNOWN;
    }

    // alg の内容を取得
    ret = cbor_value_map_find_value(val, "alg", &alg);
    if (ret != CborNoError) {
        return ret;
    }
    ret = cbor_value_get_int_checked(&alg, (int *)&cred_param->COSEAlgorithmIdentifier);
    if (ret != CborNoError) {
        return ret;
    }

    return CborNoError;
}

static int pub_key_cred_param_supported(CTAP_PUBKEY_CRED_PARAM_T *cred_param)
{
    if (cred_param->publicKeyCredentialType == PUB_KEY_CRED_PUB_KEY) {
        if (cred_param->COSEAlgorithmIdentifier == COSE_ALG_ES256) {
            return CREDENTIAL_IS_SUPPORTED;
        }
    }
    return CREDENTIAL_NOT_SUPPORTED;
}

uint8_t parse_pub_key_cred_params(CTAP_PUBKEY_CRED_PARAM_T *cred_param, CborValue *val)
{
    size_t    arr_length;
    CborError ret;
    uint8_t   i;
    CborValue array;

    // アルゴリズムサポートチェック用
    uint8_t supported_cnt = 0;
    uint8_t unsupported_cnt = 0;

    if (cbor_value_get_type(val) != CborArrayType) {
        return CTAP2_ERR_INVALID_CBOR_TYPE;
    }

    ret = cbor_value_enter_container(val, &array);
    if (ret != CborNoError) {
        return CTAP2_ERR_CBOR_PARSING;
    }

    ret = cbor_value_get_array_length(val, &arr_length);
    if (ret != CborNoError) {
        return CTAP2_ERR_CBOR_PARSING;
    }

    for (i = 0; i < arr_length; i++) {
        // 内容チェックを先に行う
        ret = check_pub_key_cred_param(&array);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }

        // 内容の解析と取得
        ret = parse_pub_key_cred_param(&array, cred_param);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }

        // アルゴリズムのサポートチェック
        if (pub_key_cred_param_supported(cred_param) == CREDENTIAL_NOT_SUPPORTED) {
            unsupported_cnt++;
        } else {
            supported_cnt++;
        }

        ret = cbor_value_advance(&array);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }
    }

    if (supported_cnt == 0 && unsupported_cnt > 0) {
        // サポートされないアルゴリズムしか存在しない場合は、
        // その旨のエラーステータスをレスポンス
        return CTAP2_ERR_UNSUPPORTED_ALGORITHM;
    }

    return CTAP1_ERR_SUCCESS;
}

uint8_t parse_options(CTAP_OPTIONS_T *options, CborValue *val, bool makeCredential)
{
    CborError ret;
    CborValue map;
    size_t    map_length;
    uint8_t   i;
    char      key[16];
    size_t    sz;
    bool      b;

    if (cbor_value_get_type(val) != CborMapType) {
        return CTAP2_ERR_INVALID_CBOR_TYPE;
    }

    ret = cbor_value_enter_container(val, &map);
    if (ret != CborNoError) {
        return CTAP2_ERR_CBOR_PARSING;
    }

    ret = cbor_value_get_map_length(val, &map_length);
    if (ret != CborNoError) {
        return CTAP2_ERR_CBOR_PARSING;
    }

    for (i = 0; i < map_length; i++) {
        if (cbor_value_get_type(&map) != CborTextStringType) {
            return CTAP2_ERR_INVALID_CBOR_TYPE;
        }
        sz = sizeof(key);
        ret = cbor_value_copy_text_string(&map, key, &sz, NULL);

        if (ret == CborErrorOutOfMemory) {
            return CTAP2_ERR_LIMIT_EXCEEDED;
        }
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }
        key[sizeof(key) - 1] = 0;

        ret = cbor_value_advance(&map);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }

        if (cbor_value_get_type(&map) != CborBooleanType) {
            return CTAP2_ERR_INVALID_CBOR_TYPE;
        }

        if (strncmp(key, "rk", 2) == 0) {
            ret = cbor_value_get_boolean(&map, &b);
            if (ret != CborNoError) {
                return CTAP2_ERR_CBOR_PARSING;
            }
            options->rk = b;

        } else if (strncmp(key, "uv", 2) == 0) {
            ret = cbor_value_get_boolean(&map, &b);
            if (ret != CborNoError) {
                return CTAP2_ERR_CBOR_PARSING;
            }
            options->uv = b;

        } else if (strncmp(key, "up", 2) == 0) {
            ret = cbor_value_get_boolean(&map, &b);
            if (ret != CborNoError) {
                return CTAP2_ERR_CBOR_PARSING;
            }

            if (makeCredential && b) {
                // makeCredentialの場合、
                // up = true の指定は、
                // 無効なオプションとしてエラーを戻す
                return CTAP2_ERR_INVALID_OPTION;
            } else {
                options->up = b;
            }
        }

        ret = cbor_value_advance(&map);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }
    }
    return CTAP1_ERR_SUCCESS;
}

uint8_t parse_credential_descriptor(CborValue *arr, CTAP_CREDENTIAL_DESC_T *cred)
{
    CborError ret;
    CborValue val;
    size_t    buflen;
    char      type[PUBKEY_CRED_TYPENM_MAX_SIZE];

    if (cbor_value_get_type(arr) != CborMapType) {
        return CTAP2_ERR_INVALID_CBOR_TYPE;
    }

    ret = cbor_value_map_find_value(arr, "id", &val);
    if (ret != CborNoError) {
        return CTAP2_ERR_MISSING_PARAMETER;
    }

    if (cbor_value_get_type(&val) != CborByteStringType) {
        return CTAP2_ERR_MISSING_PARAMETER;
    }

    buflen = CREDENTIAL_ID_MAX_SIZE;
    cbor_value_copy_byte_string(&val, cred->credential_id, &buflen, NULL);
    cred->credential_id_size = buflen;

    ret = cbor_value_map_find_value(arr, "type", &val);
    if (ret != CborNoError) {
        return CTAP2_ERR_MISSING_PARAMETER;
    }

    if (cbor_value_get_type(&val) != CborTextStringType) {
        return CTAP2_ERR_MISSING_PARAMETER;
    }

    buflen = sizeof(type);
    cbor_value_copy_text_string(&val, type, &buflen, NULL);

    if (strncmp(type, "public-key", 11) == 0) {
        cred->type = PUB_KEY_CRED_PUB_KEY;
    } else {
        cred->type = PUB_KEY_CRED_UNKNOWN;
    }

    return CTAP1_ERR_SUCCESS;
}

uint8_t parse_verify_exclude_list(CborValue *val)
{
    CborError ret;
    size_t    size;
    CborValue arr;
    uint8_t   i;
    CTAP_CREDENTIAL_DESC_T cred;

    if (cbor_value_get_type(val) != CborArrayType) {
        return CTAP2_ERR_INVALID_CBOR_TYPE;
    }

    ret = cbor_value_get_array_length(val, &size);
    if (ret != CborNoError) {
        return CTAP2_ERR_CBOR_PARSING;
    }

    ret = cbor_value_enter_container(val, &arr);
    if (ret != CborNoError) {
        return CTAP2_ERR_CBOR_PARSING;
    }

#if NRF_LOG_DEBUG_CRED_DESC
    NRF_LOG_DEBUG("previous credential id:");
    NRF_LOG_HEXDUMP_DEBUG(ctap2_pubkey_credential_id(), ctap2_pubkey_credential_id_size());
#endif

    for (i = 0; i < size; i++) {
        ret = parse_credential_descriptor(&arr, &cred);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }

#if NRF_LOG_DEBUG_CRED_DESC
    NRF_LOG_DEBUG("credential descriptor %d:", i);
    NRF_LOG_HEXDUMP_DEBUG(cred.credential_id, cred.credential_id_size);
#endif

        // 以前RegisterしたIDが除外リストに入っていた場合、
        // CTAP2_ERR_CREDENTIAL_EXCLUDED
        // をレスポンスする
        if (memcmp(cred.credential_id, ctap2_pubkey_credential_id(), ctap2_pubkey_credential_id_size()) == 0) {
            return CTAP2_ERR_CREDENTIAL_EXCLUDED;
        }

        ret = cbor_value_advance(&arr);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }
    }
    return CTAP1_ERR_SUCCESS;
}

uint8_t parse_allow_list(CTAP_ALLOW_LIST_T *allowList, CborValue *it)
{
    CborError ret;
    CborValue arr;
    size_t    len;
    uint8_t   i;

    if (cbor_value_get_type(it) != CborArrayType) {
        return CTAP2_ERR_INVALID_CBOR_TYPE;
    }

    ret = cbor_value_enter_container(it, &arr);
    if (ret != CborNoError) {
        return CTAP2_ERR_CBOR_PARSING;
    }

    ret = cbor_value_get_array_length(it, &len);
    if (ret != CborNoError) {
        return CTAP2_ERR_CBOR_PARSING;
    }

    allowList->size = 0;
    for(i = 0; i < len; i++) {
        if (i >= ALLOW_LIST_MAX_SIZE) {
            return CTAP2_ERR_TOO_MANY_ELEMENTS;
        }
        allowList->size += 1;

        ret = parse_credential_descriptor(&arr, &(allowList->list[i]));
        if (ret != CTAP1_ERR_SUCCESS) {
            return ret;
        }

        ret = cbor_value_advance(&arr);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }

    }

    return CTAP1_ERR_SUCCESS;
}

uint8_t parse_cose_pubkey(CborValue *it, CTAP_COSE_KEY *cose_key)
{
    CborValue map;
    size_t map_length;
    int i, ret, key;
    cose_key->kty = 0;
    cose_key->crv = 0;

    // 必須項目チェック済みフラグを初期化
    uint8_t must_item_flag = 0;
    
    CborType type = cbor_value_get_type(it);
    if (type != CborMapType) {
        return CTAP2_ERR_INVALID_CBOR_TYPE;
    }

    ret = cbor_value_enter_container(it,&map);
    if (ret != CborNoError) {
        return CTAP2_ERR_CBOR_PARSING;
    }

    ret = cbor_value_get_map_length(it, &map_length);
    if (ret != CborNoError) {
        return CTAP2_ERR_CBOR_PARSING;
    }

    for (i = 0; i < map_length; i++) {
        if (cbor_value_get_type(&map) != CborIntegerType) {
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
            case COSE_KEY_LABEL_KTY:
                if (cbor_value_get_type(&map) != CborIntegerType) {
                    return CTAP2_ERR_INVALID_CBOR_TYPE;
                }
                ret = cbor_value_get_int_checked(&map, &cose_key->kty);
                if (ret != CborNoError) {
                    return CTAP2_ERR_CBOR_PARSING;
                }
                must_item_flag |= 0x01;
                break;
            case COSE_KEY_LABEL_ALG:
                if (cbor_value_get_type(&map) != CborIntegerType) {
                    return CTAP2_ERR_INVALID_CBOR_TYPE;
                }
                ret = cbor_value_get_int_checked(&map, &cose_key->alg);
                if (ret != CborNoError) {
                    return CTAP2_ERR_CBOR_PARSING;
                }
                must_item_flag |= 0x02;
                break;
            case COSE_KEY_LABEL_CRV:
                if (cbor_value_get_type(&map) != CborIntegerType) {
                    return CTAP2_ERR_INVALID_CBOR_TYPE;
                }
                ret = cbor_value_get_int_checked(&map, &cose_key->crv);
                if (ret != CborNoError) {
                    return CTAP2_ERR_CBOR_PARSING;
                }
                must_item_flag |= 0x04;
                break;
            case COSE_KEY_LABEL_X:
                ret = parse_fixed_byte_string(&map, cose_key->key.x, 32);
                if (ret != CTAP1_ERR_SUCCESS) {
                    return ret;
                }
                must_item_flag |= 0x08;
                break;
            case COSE_KEY_LABEL_Y:
                ret = parse_fixed_byte_string(&map, cose_key->key.y, 32);
                if (ret != CTAP1_ERR_SUCCESS) {
                    return ret;
                }
                must_item_flag |= 0x10;
                break;
            default:
                break;
        }

        ret = cbor_value_advance(&map);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }
    }

    // 必須項目が揃っていない場合はエラー
    if (must_item_flag != 0x1f) {
        return CTAP2_ERR_MISSING_PARAMETER;
    }

    return CTAP1_ERR_SUCCESS;
}

uint8_t encode_cose_pubkey(CborEncoder *encoder, uint8_t *x, uint8_t *y, int32_t alg)
{
    CborError   ret;
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

static uint8_t parse_hmac_secret(CborValue *val, CTAP_HMAC_SECRET_T *hs)
{
    size_t    map_length;
    size_t    salt_len;
    uint8_t   parsed_count = 0;
    int       key;
    int       ret;
    uint8_t   i;
    CborValue map;

    if (cbor_value_get_type(val) != CborMapType) {
        return CTAP2_ERR_INVALID_CBOR_TYPE;
    }

    ret = cbor_value_enter_container(val,&map);
    if (ret != CborNoError) {
        return CTAP2_ERR_CBOR_PARSING;
    }

    ret = cbor_value_get_map_length(val, &map_length);
    if (ret != CborNoError) {
        return CTAP2_ERR_CBOR_PARSING;
    }

    for (i = 0; i < map_length; i++) {
        if (cbor_value_get_type(&map) != CborIntegerType) {
            return CTAP2_ERR_INVALID_CBOR_TYPE;
        }
        ret = cbor_value_get_int(&map, &key);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }
        ret = cbor_value_advance(&map);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }

        switch(key) {
            case 0x01:
                // keyAgreement(0x01)
                ret = parse_cose_pubkey(&map, hs->keyAgreement);
                if (ret != CborNoError) {
                    return CTAP2_ERR_CBOR_PARSING;
                }
                parsed_count++;
                break;

            case 0x02:
                // saltEnc(0x02): 
                //   Encrypt one or two salts (Called salt1 (32 bytes) and salt2 (32 bytes)) 
                //   using sharedSecret
                salt_len = 64;
                ret = cbor_value_copy_byte_string(&map, hs->saltEnc, &salt_len, NULL);
                if ((salt_len != 32 && salt_len != 64) || ret == CborErrorOutOfMemory) {
                    return CTAP1_ERR_INVALID_LENGTH;
                } else if (ret != CborNoError) {
                    return CTAP2_ERR_CBOR_PARSING;
                }
                hs->saltLen = salt_len;
                parsed_count++;
                break;

            case 0x03:
                // saltAuth(0x03): 
                //   LEFT(HMAC-SHA-256(sharedSecret, saltEnc), 16)
                salt_len = 32;
                ret = cbor_value_copy_byte_string(&map, hs->saltAuth, &salt_len, NULL);
                if (ret != CborNoError) {
                    return CTAP2_ERR_CBOR_PARSING;
                }
                parsed_count++;
                break;
        }

        ret = cbor_value_advance(&map);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }
    }

    if (parsed_count != 3) {
        return CTAP2_ERR_MISSING_PARAMETER;
    }

    return CTAP1_ERR_SUCCESS;
}

uint8_t parse_extensions(CborValue *val, CTAP_EXTENSIONS_T *ext)
{
    CborValue map;
    size_t    sz, map_length;
    char      key[16];
    int       ret;
    uint8_t   i;
    bool      b;

    if (cbor_value_get_type(val) != CborMapType) {
        return CTAP2_ERR_INVALID_CBOR_TYPE;
    }

    ret = cbor_value_enter_container(val, &map);
    if (ret != CborNoError) {
        return CTAP2_ERR_CBOR_PARSING;
    }

    ret = cbor_value_get_map_length(val, &map_length);
    if (ret != CborNoError) {
        return CTAP2_ERR_CBOR_PARSING;
    }

    for (i = 0; i < map_length; i++) {
        if (cbor_value_get_type(&map) != CborTextStringType) {
            return CTAP2_ERR_INVALID_CBOR_TYPE;
        }

        sz = sizeof(key);
        ret = cbor_value_copy_text_string(&map, key, &sz, NULL);
        if (ret == CborErrorOutOfMemory) {
            // Error, rp map key is too large. Ignoring.
            cbor_value_advance(&map);
            cbor_value_advance(&map);
            continue;
        } else if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }

        key[sizeof(key) - 1] = 0;
        ret = cbor_value_advance(&map);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }

        if (strncmp(key, "hmac-secret", 11) == 0) {
            if (cbor_value_get_type(&map) == CborBooleanType) {
                ret = cbor_value_get_boolean(&map, &b);
                if (ret != CborNoError) {
                    return CTAP2_ERR_CBOR_PARSING;
                }
                if (b) {
                    ext->hmac_secret_requested = true;
                }

            } else if (cbor_value_get_type(&map) == CborMapType) {
                ret = parse_hmac_secret(&map, ext->hmac_secret);
                if (ret != CTAP1_ERR_SUCCESS) {
                    return ret;
                }
            }
        }

        ret = cbor_value_advance(&map);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }
    }
    return CTAP1_ERR_SUCCESS;
}
