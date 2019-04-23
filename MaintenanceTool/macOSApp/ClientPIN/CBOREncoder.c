//
//  CBOREncoder.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/18.
//
#include "CBOREncoder.h"
#include "ECDH.h"
#include "FIDODefines.h"
#include "fido_blob.h"
#include "fido_crypto.h"
#include "debug_log.h"

// Values for COSE_Key format
//  Key type
#define COSE_KEY_LABEL_KTY  1
#define COSE_KEY_KTY_EC2    2
//  Curve type
#define COSE_KEY_LABEL_CRV  -1
#define COSE_KEY_CRV_P256   1
//  Key coordinate
#define COSE_KEY_LABEL_X    -2
#define COSE_KEY_LABEL_Y    -3
//  Signature algorithm
#define COSE_KEY_LABEL_ALG  3
#define COSE_ALG_ES256      -7

static uint8_t requestBytes[1024];
static size_t  requestBytesLength;

uint8_t *ctap2_cbor_encode_request_bytes(void) {
    return requestBytes;
}

size_t ctap2_cbor_encode_request_bytes_size(void) {
    return requestBytesLength;
}

uint8_t ctap2_cbor_encode_get_agreement_key(void) {
    // 作業領域初期化
    memset(requestBytes, 0x00, sizeof(requestBytes));
    requestBytesLength = 0;
    // encoded_buffの１バイト目にCBORコマンドを設定
    requestBytes[0] = CTAP2_CMD_CLIENT_PIN;
    // エンコード結果を格納する領域
    uint8_t *encoded_buff = (uint8_t *)requestBytes + 1;
    size_t encoded_buff_size = sizeof(requestBytes) - 1;
    // CBORエンコーダーを初期化
    CborEncoder encoder;
    cbor_encoder_init(&encoder, encoded_buff, encoded_buff_size, 0);
    // Mapに格納する要素数の設定
    size_t map_elements_num = 2;
    // Map初期化
    CborEncoder map;
    CborError ret = cbor_encoder_create_map(&encoder, &map, map_elements_num);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }
    // pinProtocol(0x01): 0x01
    ret = cbor_encode_int(&map, 0x01);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }
    ret = cbor_encode_uint(&map, 0x01);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }
    // subCommand(0x02): getKeyAgreement(0x02)
    ret = cbor_encode_int(&map, 0x02);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }
    ret = cbor_encode_uint(&map, 0x02);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }
    // Mapクローズ
    ret = cbor_encoder_close_container(&encoder, &map);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }
    // CBORバッファの長さを設定
    encoded_buff_size = cbor_encoder_get_buffer_size(&encoder, encoded_buff);
    requestBytesLength = encoded_buff_size + 1;
    return CTAP1_ERR_SUCCESS;
}

static uint8_t encode_cose_pubkey(CborEncoder *encoder, uint8_t *x, uint8_t *y, int32_t alg) {
    CborError   ret;
    CborEncoder map;
    
    // Map初期化
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
    // Mapクローズ
    ret = cbor_encoder_close_container(encoder, &map);
    if (ret != CborNoError) {
        return ret;
    }
    
    return CborNoError;
}

static uint8_t add_encoded_cosekey_to_map(CborEncoder *encoder) {
    // CBORエンコード実行
    uint8_t *x = ECDH_public_key_X();
    uint8_t *y = ECDH_public_key_Y();
    int32_t alg = COSE_ALG_ES256;
    uint8_t ret = encode_cose_pubkey(encoder, x, y, alg);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    return CTAP1_ERR_SUCCESS;
}

static uint8_t generate_set_pin_cbor(bool change_pin) {
    // Mapに格納する要素数
    size_t map_elements_num;
    // 作業領域初期化
    memset(requestBytes, 0x00, sizeof(requestBytes));
    requestBytesLength = 0;
    // encoded_buffの１バイト目にCBORコマンドを設定
    requestBytes[0] = CTAP2_CMD_CLIENT_PIN;
    // エンコード結果を格納する領域
    uint8_t *encoded_buff = (uint8_t *)requestBytes + 1;
    size_t encoded_buff_size = sizeof(requestBytes) - 1;
    // CBORエンコーダーを初期化
    CborEncoder encoder;
    cbor_encoder_init(&encoder, encoded_buff, encoded_buff_size, 0);
    // Mapに格納する要素数の設定
    if (change_pin) {
        map_elements_num = 6;
    } else {
        map_elements_num = 5;
    }
    // Map初期化
    CborEncoder map;
    CborError ret = cbor_encoder_create_map(&encoder, &map, map_elements_num);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // pinProtocol(0x01): 0x01
    ret = cbor_encode_int(&map, 0x01);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = cbor_encode_uint(&map, 0x01);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // subCommand(0x02)
    ret = cbor_encode_int(&map, 0x02);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    if (change_pin) {
        ret = cbor_encode_uint(&map, CTAP2_SUBCMD_CLIENT_PIN_CHANGE);
    } else {
        ret = cbor_encode_uint(&map, CTAP2_SUBCMD_CLIENT_PIN_SET);
    }
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // keyAgreement(0x03)
    ret = cbor_encode_int(&map, 0x03);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = add_encoded_cosekey_to_map(&map);
    if (ret != CTAP1_ERR_SUCCESS) {
        return ret;
    }
    // pinAuth(0x04) 16 bytes
    ret = cbor_encode_int(&map, 0x04);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = cbor_encode_byte_string(&map, pin_auth(), 16);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // newPinEnc(0x05)
    ret = cbor_encode_int(&map, 0x05);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = cbor_encode_byte_string(&map, new_pin_enc(), new_pin_enc_size());
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // pinHashEnc(0x06) 16 bytes
    if (change_pin) {
        ret = cbor_encode_int(&map, 0x06);
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
        ret = cbor_encode_byte_string(&map, pin_hash_enc(), 16);
        if (ret != CborNoError) {
            return CTAP1_ERR_OTHER;
        }
    }
    // Mapクローズ
    ret = cbor_encoder_close_container(&encoder, &map);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // CBORバッファの長さを設定
    encoded_buff_size = cbor_encoder_get_buffer_size(&encoder, encoded_buff);
    requestBytesLength = encoded_buff_size + 1;
    return CTAP1_ERR_SUCCESS;
}

uint8_t ctap2_cbor_encode_client_pin_set_or_change(
    uint8_t *agreement_pubkey_X, uint8_t *agreement_pubkey_Y, char *new_pin, char *old_pin) {
    // ECDHキーペアを新規作成し、受領した公開鍵から共通鍵を生成
    if (ECDH_create_shared_secret_key(agreement_pubkey_X, agreement_pubkey_Y) != CTAP1_ERR_SUCCESS) {
        return CTAP1_ERR_OTHER;
    }
    // pinHashEncを生成
    bool change_pin = (old_pin != NULL);
    if (change_pin) {
        if (generate_pin_hash_enc(old_pin) != CTAP1_ERR_SUCCESS) {
            return CTAP1_ERR_OTHER;
        }
    }
    // newPinEncを生成
    if (generate_new_pin_enc(new_pin) != CTAP1_ERR_SUCCESS) {
        return CTAP1_ERR_OTHER;
    }
    // pinAuthを生成
    if (generate_pin_auth(change_pin) != CTAP1_ERR_SUCCESS) {
        return CTAP1_ERR_OTHER;
    }
    // リクエストCBORを生成
    return generate_set_pin_cbor(change_pin);
}
