//
//  mcumgr_cbor_decode.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/11/02.
//
#include <stdio.h>
#include "cbor.h"
#include "debug_log.h"

#define HASH_SIZE   32
#define SLOT_CNT    2

typedef struct _slot_info {
    uint8_t slot_no;
    uint8_t hash_bytes[HASH_SIZE];
    bool    active;
} SLOT_INFO;

typedef struct _result_info {
    uint8_t  rc;
} RESULT_INFO;

static SLOT_INFO   slot_infos[SLOT_CNT];
static RESULT_INFO result_info;

uint8_t *mcumgr_cbor_decode_slot_info_hash(int slot_no)
{
    if (slot_infos[slot_no].slot_no == slot_no) {
        return slot_infos[slot_no].hash_bytes;
    } else {
        return NULL;
    }
}

bool mcumgr_cbor_decode_slot_info_active(int slot_no)
{
    if (slot_infos[slot_no].slot_no == slot_no) {
        return slot_infos[slot_no].active;
    } else {
        return false;
    }
}

uint8_t mcumgr_cbor_decode_result_info_rc(void)
{
    return result_info.rc;
}

static bool parse_integer_value(const CborValue *map, const char *string, int *result)
{
    // Mapから指定キーのエントリーを抽出
    CborValue value;
    CborError ret = cbor_value_map_find_value(map, string, &value);
    if (ret != CborNoError) {
        log_debug("%s: cbor_value_map_find_value(%s) returns %d", __func__, string, ret);
        return false;
    }
    // 型をチェック
    CborType type = cbor_value_get_type(&value);
    if (type != CborIntegerType) {
        log_debug("%s: cbor_value_get_type(%s) returns type %d", __func__, string, type);
        return false;
    }
    // 値を抽出
    ret = cbor_value_get_int_checked(&value, result);
    if (ret != CborNoError) {
        log_debug("%s: cbor_value_get_int_checked(%s) returns %d", __func__, string, ret);
        return false;
    }
    return true;
}

static bool parse_fixed_bytes_value(const CborValue *map, const char *string, uint8_t *result, size_t size)
{
    // Mapから指定キーのエントリーを抽出
    CborValue value;
    CborError ret = cbor_value_map_find_value(map, string, &value);
    if (ret != CborNoError) {
        log_debug("%s: cbor_value_map_find_value(%s) returns %d", __func__, string, ret);
        return false;
    }
    // 型をチェック
    CborType type = cbor_value_get_type(&value);
    if (type != CborByteStringType) {
        log_debug("%s: cbor_value_get_type(%s) returns type %d", __func__, string, type);
        return false;
    }
    // 値を抽出
    size_t sz = size;
    ret = cbor_value_copy_byte_string(&value, result, &sz, NULL);
    if (ret != CborNoError) {
        log_debug("%s: cbor_value_copy_byte_string(%s) returns %d", __func__, string, ret);
        return false;
    }
    // 抽出サイズをチェック
    if (sz != size) {
        log_debug("%s: cbor_value_copy_byte_string(%s) returns size %d", __func__, string, sz);
        return false;
    }
    return true;
}

static bool parse_boolean_value(const CborValue *map, const char *string, bool *result)
{
    // Mapから指定キーのエントリーを抽出
    CborValue value;
    CborError ret = cbor_value_map_find_value(map, string, &value);
    if (ret != CborNoError) {
        log_debug("%s: cbor_value_map_find_value(%s) returns %d", __func__, string, ret);
        return false;
    }
    // 型をチェック
    CborType type = cbor_value_get_type(&value);
    if (type != CborBooleanType) {
        log_debug("%s: cbor_value_get_type(%s) returns type %d", __func__, string, type);
        return false;
    }
    // 値を抽出
    ret = cbor_value_get_boolean(&value, result);
    if (ret != CborNoError) {
        log_debug("%s: cbor_value_get_boolean(%s) returns %d", __func__, string, ret);
        return false;
    }
    return true;
}

static void mcumgr_cbor_decode_slot_info_init(void)
{
    // 構造体を初期化
    size_t size = sizeof(SLOT_INFO) * SLOT_CNT;
    memset(slot_infos, 0, size);
    memset(&result_info, 0, sizeof(RESULT_INFO));
}

static bool parse_root_map(const uint8_t *buffer, size_t size, CborParser *parser, CborValue *root_map)
{
    // CBOR parser初期化
    CborError ret = cbor_parser_init(buffer, size, CborValidateCanonicalFormat, parser, root_map);
    if (ret != CborNoError) {
        log_debug("%s: cbor_parser_init returns %d", __func__, ret);
        return false;
    }
    // ルートのMapを抽出
    CborType type = cbor_value_get_type(root_map);
    if (type != CborMapType) {
        log_debug("%s: cbor_value_get_type returns type %d", __func__, type);
        return false;
    }
    return true;
}

static bool parse_array(const CborValue *map, const char *string, CborValue *result)
{
    // Mapから指定キーのエントリーを抽出
    CborError ret = cbor_value_map_find_value(map, string, result);
    if (ret != CborNoError) {
        log_debug("%s: cbor_value_map_find_value(%s) returns %d", __func__, string, ret);
        return false;
    }
    // 型をチェック
    CborType type = cbor_value_get_type(result);
    if (type != CborArrayType) {
        log_debug("%s: cbor_value_get_type(%s) returns type %d", __func__, string, type);
        return false;
    }
    return true;
}

static bool parse_images_array(const CborValue *array)
{
    // 配列内を探索
    CborValue map;
    CborError ret = cbor_value_enter_container(array, &map);
    if (ret != CborNoError) {
        log_debug("%s: cbor_value_enter_container returns %d", __func__, ret);
        return false;
    }
    while (ret == CborNoError) {
        // break byteを検出したらループ脱出、配列要素がMapでない場合はエラー
        CborType type = cbor_value_get_type(&map);
        if (type == CborInvalidType) {
            break;
        } else if (type != CborMapType) {
            log_debug("%s: cbor_value_get_type returns type %d", __func__, type);
            return false;
        }
        // "slot"エントリーを抽出（数値）
        int slot;
        if (parse_integer_value(&map, "slot", &slot) == false) {
            return false;
        }
        slot_infos[slot].slot_no = slot;
        // "hash"エントリーを抽出（バイト配列）
        if (parse_fixed_bytes_value(&map, "hash", slot_infos[slot].hash_bytes, HASH_SIZE) == false) {
            return false;
        }
        // "active"エントリーを抽出（bool）
        if (parse_boolean_value(&map, "active", &slot_infos[slot].active) == false) {
            return false;
        }
        // 次の配列要素に移動
        ret = cbor_value_advance(&map);
        if (ret != CborNoError) {
            log_debug("%s: cbor_value_advance returns %d", __func__, ret);
            return false;
        }
    }
    return true;
}

static bool parse_rc(const CborValue *map)
{
    // "rc"エントリーを抽出（数値）
    int rc;
    if (parse_integer_value(map, "rc", &rc) == false) {
        return false;
    }
    result_info.rc = (uint8_t)rc;
    return true;
}

bool mcumgr_cbor_decode_slot_info(uint8_t *cbor_data_buffer, size_t cbor_data_length)
{
    // 初期処理
    mcumgr_cbor_decode_slot_info_init();
    // ルートのMapを抽出
    CborParser parser;
    CborValue root_map;
    if (parse_root_map(cbor_data_buffer, cbor_data_length, &parser, &root_map) == false) {
        return false;
    }
    // "images"エントリーを抽出（配列）
    CborValue array;
    if (parse_array(&root_map, "images", &array)) {
        if (parse_images_array(&array) == false) {
            return false;
        }
    } else {
        // "images"がない場合は、代わりに"rc"を抽出
        if (parse_rc(&root_map) == false) {
            return false;
        }
    }
    // 正常終了
    return true;
}
