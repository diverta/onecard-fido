//
//  triple_des.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/28.
//
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "debug_log.h"
#include "tool_crypto_common.h"

// DES鍵を保持
static uint8_t des_key[DES_LEN_3DES];

bool triple_des_import_key(const unsigned char *key_raw, const size_t key_raw_size)
{
    // パラメーターチェック
    if (!key_raw) {
        log_debug("%s: DES parameter is NULL", __func__);
        return false;
    }
    if (key_raw_size != DES_LEN_3DES) {
        log_debug("%s: DES invalid key size", __func__);
        return false;
    }
    // 3DESキーの内容をバッファにコピー
    memcpy(des_key, key_raw, key_raw_size);
    return true;
}

bool triple_des_decrypt(const unsigned char *input, const size_t input_size, unsigned char *output, size_t *output_size)
{
    return false;
}
