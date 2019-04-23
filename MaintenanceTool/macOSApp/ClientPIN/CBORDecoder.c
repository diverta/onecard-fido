//
//  CBORDecoder.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/22.
//
#include "CBORDecoder.h"
#include "FIDODefines.h"

static CTAP_COSE_KEY cose_key;

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
