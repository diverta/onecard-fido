/* 
 * File:   ctap2_cbor_encode.c
 * Author: makmorit
 *
 * Created on 2019/02/18, 15:31
 */
#include <stdio.h>
#include <string.h>

#include "fido_command_common.h"
#include "fido_common.h"
#include "ctap2_cbor.h"
#include "ctap2_cbor_define.h"
#include "ctap2_client_pin_token.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

uint8_t ctap2_cbor_encode_cose_pubkey(CborEncoder *encoder, uint8_t *x, uint8_t *y, int32_t alg)
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

static uint8_t add_encoded_cosekey_to_map(CborEncoder *encoder)
{
    // CBORエンコード実行
    uint8_t *x = fido_command_sskey_public_key();
    uint8_t *y = fido_command_sskey_public_key() + 32;
    int32_t alg = COSE_ALG_ES256;
    uint8_t ret = ctap2_cbor_encode_cose_pubkey(encoder, x, y, alg);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    return CTAP1_ERR_SUCCESS;
}

uint8_t ctap2_cbor_encode_response_retry_counter(uint8_t *encoded_buff, size_t *encoded_buff_size, uint32_t retry_counter)
{
    // 作業領域初期化
    memset(encoded_buff, 0x00, *encoded_buff_size);
    
    // CBORエンコーダーを初期化
    CborEncoder encoder;
    ctap2_cbor_encoder_init(&encoder, encoded_buff, *encoded_buff_size, 0);

    // Mapに格納する要素数の設定
    size_t map_elements_num = 1;

    // Map初期化
    CborEncoder map;
    CborError ret = cbor_encoder_create_map(&encoder, &map, map_elements_num);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    // retries (0x03)
    ret = cbor_encode_int(&map, 0x03);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // リトライカウンター値をエンコードし、
    // mapにセット
    ret = cbor_encode_uint(&map, retry_counter);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    ret = cbor_encoder_close_container(&encoder, &map);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    // CBORバッファの長さを設定
    *encoded_buff_size = ctap2_cbor_encoder_get_buffer_size(&encoder, encoded_buff);

    return CTAP1_ERR_SUCCESS;
}

uint8_t ctap2_cbor_encode_response_key_agreement(uint8_t *encoded_buff, size_t *encoded_buff_size)
{
    // 作業領域初期化
    memset(encoded_buff, 0x00, *encoded_buff_size);
    
    // CBORエンコーダーを初期化
    CborEncoder encoder;
    ctap2_cbor_encoder_init(&encoder, encoded_buff, *encoded_buff_size, 0);

    // Mapに格納する要素数の設定
    size_t map_elements_num = 1;

    // Map初期化
    CborEncoder map;
    CborError ret = cbor_encoder_create_map(&encoder, &map, map_elements_num);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    // KeyAgreement (0x01)
    ret = cbor_encode_int(&map, 0x01);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    // 生成された公開鍵をCOSE形式にエンコードし、
    // mapにセット
    uint8_t err = add_encoded_cosekey_to_map(&map);
    if (err != CTAP1_ERR_SUCCESS) {
        return err;
    }

    ret = cbor_encoder_close_container(&encoder, &map);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    // CBORバッファの長さを設定
    *encoded_buff_size = ctap2_cbor_encoder_get_buffer_size(&encoder, encoded_buff);

    return CTAP1_ERR_SUCCESS;
}

uint8_t ctap2_cbor_encode_response_set_pin(uint8_t *encoded_buff, size_t *encoded_buff_size)
{
    // 作業領域初期化
    memset(encoded_buff, 0x00, *encoded_buff_size);
    
    // CBORエンコーダーを初期化
    CborEncoder encoder;
    ctap2_cbor_encoder_init(&encoder, encoded_buff, *encoded_buff_size, 0);

    // CBORバッファの長さを設定
    *encoded_buff_size = ctap2_cbor_encoder_get_buffer_size(&encoder, encoded_buff);

    return CTAP1_ERR_SUCCESS;
}

uint8_t ctap2_cbor_encode_response_get_pin_token(uint8_t *encoded_buff, size_t *encoded_buff_size)
{
    // 作業領域初期化
    memset(encoded_buff, 0x00, *encoded_buff_size);
    
    // CBORエンコーダーを初期化
    CborEncoder encoder;
    ctap2_cbor_encoder_init(&encoder, encoded_buff, *encoded_buff_size, 0);

    // Mapに格納する要素数の設定
    size_t map_elements_num = 1;

    // Map初期化
    CborEncoder map;
    CborError ret = cbor_encoder_create_map(&encoder, &map, map_elements_num);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    // pinToken (0x02)
    ret = cbor_encode_int(&map, 0x02);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }
    ret = cbor_encode_byte_string(&map, ctap2_client_pin_token_encoded(), ctap2_client_pin_token_encoded_size());
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    ret = cbor_encoder_close_container(&encoder, &map);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    // CBORバッファの長さを設定
    *encoded_buff_size = ctap2_cbor_encoder_get_buffer_size(&encoder, encoded_buff);

    return CTAP1_ERR_SUCCESS;
}
