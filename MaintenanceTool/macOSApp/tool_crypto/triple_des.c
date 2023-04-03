//
//  triple_des.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/28.
//
#import <CommonCrypto/CommonCryptor.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "debug_log.h"

// DES鍵を保持
static uint8_t des_key[kCCKeySize3DES];

bool triple_des_import_key(const unsigned char *key_raw, const size_t key_raw_size)
{
    // パラメーターチェック
    if (!key_raw) {
        log_debug("%s: DES parameter is NULL", __func__);
        return false;
    }
    if (key_raw_size != kCCKeySize3DES) {
        log_debug("%s: DES invalid key size", __func__);
        return false;
    }
    // 3DESキーの内容をバッファにコピー
    memcpy(des_key, key_raw, key_raw_size);
    return true;
}

bool triple_des_decrypt(const unsigned char *input, const size_t input_size, unsigned char *output, size_t *output_size)
{
    bool ret = false;

    // Allocate buffer for decryption
    size_t size = input_size;
    void  *buff = malloc(size);

    // Perform decryption
    CCCryptorStatus result = CCCrypt(kCCDecrypt, kCCAlgorithm3DES, 0, des_key, sizeof(des_key), NULL, input, input_size, buff, size, output_size);
    // Copy decrypted bytes
    if (result == kCCSuccess) {
        memcpy(output, buff, *output_size);
        ret = true;
    } else {
        log_debug("%s fail: CCCryptorStatus=%d", __func__, result);
    }

    // Deallocate buffer
    free(buff);
    return ret;
}
