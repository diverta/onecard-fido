//
//  CBORDecoder.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/22.
//
#include "CBORDecoder.h"
#include "FIDODefines.h"
#include "fido_crypto.h"

// 公開鍵格納領域
static CTAP_COSE_KEY cose_key;

// PINトークン格納領域
#define PIN_TOKEN_SIZE 16
static uint8_t decrypted_pin_token[PIN_TOKEN_SIZE];

// 暗号化されたPINトークンの格納領域
static uint8_t encrypted_pin_token[PIN_TOKEN_SIZE];

// MakeCredentialレスポンス格納領域
static CTAP_MAKE_CREDENTIAL_RES make_credential_res;

static uint8_t parse_fixed_byte_string(CborValue *map, uint8_t *dst, int len)
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

static uint8_t parse_cose_pubkey(CborValue *it, CTAP_COSE_KEY *cose_key)
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

uint8_t ctap2_cbor_decode_get_agreement_key(uint8_t *cbor_data_buffer, size_t cbor_data_length) {
    CborParser  parser;
    CborValue   it;
    CborValue   map;
    size_t      map_length;
    CborType    type;
    CborError   ret;
    uint8_t     i;
    int         key;

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
                // keyAgreement (COSE_Key)
                ret = parse_cose_pubkey(&map, &cose_key);
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

    return CTAP1_ERR_SUCCESS;
}

uint8_t *ctap2_cbor_decode_agreement_pubkey_X(void) {
    return cose_key.key.x;
}

uint8_t *ctap2_cbor_decode_agreement_pubkey_Y(void) {
    return cose_key.key.y;
}

uint8_t ctap2_cbor_decode_encrypted_pin_token(uint8_t *cbor_data_buffer, size_t cbor_data_length) {
    CborParser  parser;
    CborValue   it;
    CborValue   map;
    size_t      map_length;
    CborType    type;
    CborError   ret;
    uint8_t     i;
    int         key;
    
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
            case 2:
                // pinToken (Byte Array)
                ret = parse_fixed_byte_string(&map, encrypted_pin_token, PIN_TOKEN_SIZE);
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
    
    return CTAP1_ERR_SUCCESS;
}

uint8_t ctap2_cbor_decode_pin_token(uint8_t *cbor_data_buffer, size_t cbor_data_length) {
    // 暗号化されたPINトークンをCBORから抽出
    uint8_t ret = ctap2_cbor_decode_encrypted_pin_token(cbor_data_buffer, cbor_data_length);
    if (ret != CTAP1_ERR_SUCCESS) {
        return ret;
    }
    // PINトークンを復号化
    ret = decrypto_pin_token(encrypted_pin_token, decrypted_pin_token, PIN_TOKEN_SIZE);
    if (ret != CTAP1_ERR_SUCCESS) {
        return ret;
    }

    return CTAP1_ERR_SUCCESS;
}

uint8_t *ctap2_cbor_decrypted_pin_token(void) {
    return decrypted_pin_token;
}

uint8_t parse_auth_data(uint8_t *auth_data_bytes, size_t auth_data_size) {
    // rpIdHash
    uint8_t index = 0;
    memcpy(make_credential_res.rpIdHash, auth_data_bytes + index, RP_ID_HASH_SIZE);
    index += RP_ID_HASH_SIZE;

    // flags
    make_credential_res.flags = auth_data_bytes[index];
    index++;

    // signCount（エンディアン変換が必要）
    uint8_t *sign_count = (uint8_t *)&make_credential_res.signCount;
    for (int j = 0; j < SIGN_COUNT_SIZE; j++) {
        sign_count[SIGN_COUNT_SIZE - 1 - j] = auth_data_bytes[index + j];
    }
    index += SIGN_COUNT_SIZE;

    // aaguid
    memcpy(make_credential_res.aaguid, auth_data_bytes + index, AAGUID_SIZE);
    index += AAGUID_SIZE;

    // credentialIdLength（エンディアン変換が必要）
    uint8_t *id_length = (uint8_t *)&make_credential_res.credentialIdLength;
    for (int j = 0; j < CREDENTIAL_ID_LENGTH_SIZE; j++) {
        id_length[CREDENTIAL_ID_LENGTH_SIZE - 1 - j] = auth_data_bytes[index + j];
    }
    index += CREDENTIAL_ID_LENGTH_SIZE;

    // CredentialId
    memcpy(make_credential_res.credentialId, auth_data_bytes + index,
           make_credential_res.credentialIdLength);
    index += make_credential_res.credentialIdLength;

    // Credential Public Key
    memcpy(make_credential_res.credentialPubKey, auth_data_bytes + index,
           CREDENTIAL_PUBKEY_MAX_SIZE);
    index += CREDENTIAL_PUBKEY_MAX_SIZE;

    if (auth_data_size - index < EXT_CBOR_FOR_CRED_MAX_SIZE) {
        return CTAP1_ERR_SUCCESS;
    }

    // Extensions CBOR for makeCredential
    memcpy(make_credential_res.extCBORForCred, auth_data_bytes + index,
           EXT_CBOR_FOR_CRED_MAX_SIZE);

    return CTAP1_ERR_SUCCESS;
}

uint8_t ctap2_cbor_decode_make_credential(uint8_t *cbor_data_buffer, size_t cbor_data_length) {
    CborParser  parser;
    CborValue   it;
    CborValue   map;
    size_t      map_length;
    CborType    type;
    CborError   ret;
    uint8_t     i;
    int         key;

    // authData格納領域
    uint8_t auth_data_bytes[256];
    size_t  auth_data_size;

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
            case 0x02:
                // authData (Byte Array)
                if (cbor_value_get_type(&map) != CborByteStringType) {
                    return CTAP2_ERR_INVALID_CBOR_TYPE;
                }
                auth_data_size = sizeof(auth_data_bytes);
                ret = cbor_value_copy_byte_string(&map, auth_data_bytes, &auth_data_size, NULL);
                if (ret == CborErrorOutOfMemory) {
                    return CTAP2_ERR_LIMIT_EXCEEDED;
                }
                if (ret != CborNoError) {
                    return ret;
                }
                ret = parse_auth_data(auth_data_bytes, auth_data_size);
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
    
    return CTAP1_ERR_SUCCESS;
}

uint8_t *ctap2_cbor_decode_credential_id(void) {
    return make_credential_res.credentialId;
}

size_t ctap2_cbor_decode_credential_id_size(void) {
    return make_credential_res.credentialIdLength;
}
