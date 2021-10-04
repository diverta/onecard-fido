/* 
 * File:   fido_maintenance_cryption.c
 * Author: makmorit
 *
 * Created on 2019/12/09, 12:01
 */
//
// プラットフォーム非依存コード
//
#include "ctap2_cbor.h"
#include "ctap2_cbor_parse.h"
#include "ctap2_common.h"
#include "fido_command_common.h"
#include "fido_common.h"
#include "fido_ctap2_command.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(fido_maintenance_cryption);
#endif

// for debug data
#define LOG_DEBUG_CBOR_REQUEST      false
#define LOG_DEBUG_CRYPTION_DATA     false

// 暗号化データ読込用の作業領域
#define CRYPTION_DATA_MAX_SIZE 1024
static uint8_t cryption_data[CRYPTION_DATA_MAX_SIZE];
static size_t  cryption_size;

// デコードされたリクエストデータを保持する構造体
static struct {
    CTAP_COSE_KEY cose_key;
    uint8_t cryption_bytes_enc[CRYPTION_DATA_MAX_SIZE];
    size_t  cryption_bytes_enc_size;
    size_t  cryption_bytes_size;
} ctap2_request;

uint8_t *fido_maintenance_cryption_data(void)
{
    return cryption_data;
}

size_t fido_maintenance_cryption_size(void)
{
    return cryption_size;
}

#if LOG_DEBUG_CBOR_REQUEST
static void debug_decoded_request()
{
    fido_log_debug("keyAgreement: alg(%d) curve(%d) public key(64 bytes):",
        ctap2_request.cose_key.alg, ctap2_request.cose_key.crv);
    fido_log_print_hexdump_debug(ctap2_request.cose_key.key.x, 32);
    fido_log_print_hexdump_debug(ctap2_request.cose_key.key.y, 32);

    fido_log_debug("cryptionBytesEnc(%d bytes):", ctap2_request.cryption_bytes_enc_size);
    int j, k;
    int max = (ctap2_request.cryption_bytes_enc_size < CRYPTION_DATA_MAX_SIZE) 
                ? ctap2_request.cryption_bytes_enc_size : CRYPTION_DATA_MAX_SIZE;
    for (j = 0; j < 128; j += 64) {
        k = max - j;
        fido_log_print_hexdump_debug(ctap2_request.cryption_bytes_enc + j, (k < 64) ? k : 64);
    }

    fido_log_debug("cryptionBytesSize(%d bytes)", ctap2_request.cryption_bytes_size);
}
#endif

static uint8_t decode_request_cbor(uint8_t *cbor_data_buffer, size_t cbor_data_length)
{
    CborParser  parser;
    CborValue   it;
    CborValue   map;
    size_t      map_length;
    CborType    type;
    CborError   ret;
    uint8_t     err;
    uint8_t     i;
    int         key;
    size_t      sz;

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
                // keyAgreement (COSE_Key)
                err = parse_cose_pubkey(&map, &ctap2_request.cose_key);
                if (err != CTAP1_ERR_SUCCESS) {
                    return err;
                }
                must_item_flag |= 0x01;
                break;
            case 2:
                // cryptionBytesEnc (Variable length Byte Array)
                if (cbor_value_get_type(&map) != CborByteStringType) {
                    return CTAP2_ERR_INVALID_CBOR_TYPE;
                }
                if (cbor_value_calculate_string_length(&map, &sz) != CborNoError) {
                    return CTAP2_ERR_CBOR_PARSING;
                }
                ctap2_request.cryption_bytes_enc_size = sz;
                sz = CRYPTION_DATA_MAX_SIZE;
                if (cbor_value_copy_byte_string(&map, ctap2_request.cryption_bytes_enc, &sz, NULL) != CborNoError) {
                    return CTAP2_ERR_CBOR_PARSING;
                }
                must_item_flag |= 0x02;
                break;
            case 3:
                // cryptionBytesSize (Unsigned Integer)
                if (cbor_value_get_type(&map) != CborIntegerType) {
                    return CTAP2_ERR_INVALID_CBOR_TYPE;
                }
                if (cbor_value_get_int_checked(&map, (int *)&ctap2_request.cryption_bytes_size) != CborNoError) {
                    return CTAP2_ERR_CBOR_PARSING;
                }
                must_item_flag |= 0x04;
                break;
            default:
                break;
        }

        ret = cbor_value_advance(&map);
        if (ret != CborNoError) {
            return CTAP2_ERR_CBOR_PARSING;
        }
    }

#if LOG_DEBUG_CBOR_REQUEST
    debug_decoded_request();
#endif

    // 必須項目が揃っていない場合はエラー
    if (must_item_flag != 0x07) {
        return CTAP2_ERR_MISSING_PARAMETER;
    }

    return CTAP1_ERR_SUCCESS;
}

bool fido_maintenance_cryption_restore(uint8_t *cbor_data_buffer, size_t cbor_data_length)
{
    // リクエストCBORから、暗号化済みデータを抽出
    uint8_t ctap2_status = decode_request_cbor(cbor_data_buffer, cbor_data_length);
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        fido_log_error("fido_maintenance_cryption_restore: failed to decode CBOR request");
        fido_ctap2_command_send_response(ctap2_status, 1);
        return false;
    }

    // 管理ツールから受け取った公開鍵と、
    // 鍵交換用キーペアの秘密鍵を使用し、共通鍵ハッシュを生成
    ctap2_status = fido_command_sskey_generate((uint8_t *)&ctap2_request.cose_key.key);
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // 鍵交換用キーペアが未生成の場合は
        // エラーレスポンスを生成して戻す
        fido_ctap2_command_send_response(ctap2_status, 1);
        return false;
    }

    // 管理ツールから受け取った暗号化済みデータを、
    // 共通鍵ハッシュを使用して復号化
    size_t cryption_bytes_enc_size = fido_command_sskey_aes_256_cbc_decrypt(
        ctap2_request.cryption_bytes_enc, ctap2_request.cryption_bytes_enc_size, cryption_data);
    if (cryption_bytes_enc_size != ctap2_request.cryption_bytes_enc_size) {
        // 処理NGの場合はエラーレスポンスを生成して戻す
        fido_ctap2_command_send_response(CTAP1_ERR_OTHER, 1);
        return false;
    }
#if LOG_DEBUG_CRYPTION_DATA
    fido_log_debug("Decrypted data(%d bytes):", cryption_bytes_enc_size);
    fido_log_print_hexdump_debug(cryption_data, 64);
#endif

    // データ長を設定
    cryption_size = ctap2_request.cryption_bytes_size;
    return true;
}
