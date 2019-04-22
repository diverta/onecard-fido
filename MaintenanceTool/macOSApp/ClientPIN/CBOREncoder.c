//
//  CBOREncoder.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/18.
//
#include "CBOREncoder.h"
#include "ECDH.h"
#include "FIDODefines.h"

static uint8_t requestBytes[1024];
static size_t  requestBytesLength;

// エラーメッセージを保持
static char *error_message;

char *CBOREncoder_error_message(void) {
    return error_message;
}

uint8_t *ctap2_cbor_encode_request_bytes(void) {
    return requestBytes;
}

size_t ctap2_cbor_encode_request_bytes_size(void) {
    return requestBytesLength;
}

uint8_t ctap2_cbor_encode_get_agreement_key(void) {
    // 作業領域初期化
    memset(requestBytes, 0x00, sizeof(requestBytes));
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
        requestBytesLength = 0;
        return CTAP2_ERR_PROCESSING;
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
        error_message = ECDH_error_message();
        return CTAP1_ERR_OTHER;
    }

    // 仮の仕様
    requestBytesLength = 0;
    return CTAP1_ERR_SUCCESS;
}
