/* 
 * File:   fido_flash.cpp
 * Author: makmorit
 *
 * Created on 2019/07/31, 14:28
 */
#include "mbed.h"
#include "nvstore.h"

#include "fido_flash.h"
#include "fido_log.h"

//
// NVStore内での管理用キー
//  0: 共有情報
//  1: AESパスワード
//  2: 秘密鍵
//  3: 証明書
//  4: リトライカウンター
//  5-99: トークンカウンター
//
#define NVSTORE_KEY_CONTEXT 0
#define NVSTORE_KEY_AESPSWD 1
#define NVSTORE_KEY_PRIVKEY 2
#define NVSTORE_KEY_CERTDAT 3
#define NVSTORE_KEY_RETRCNT 4
#define NVSTORE_KEY_TOKNCNT 5

//
// データ読込用の作業領域
//
static uint32_t user_buf[1024];

bool fido_flash_record_write(uint16_t key, void *buf, size_t size)
{
    NVStore &nvstore = NVStore::get_instance();
    int ret = nvstore.set(key, size, buf);
    fido_log_error("fido_flash_record_write: nvstore.set(%d) returns %d", key, ret);
    if (ret != NVSTORE_SUCCESS) {
        return false;
    }

    return true;
}

bool fido_flash_record_read(uint16_t key, void *buf, size_t *size)
{
    NVStore &nvstore = NVStore::get_instance();
    uint16_t actual_size;
    int ret = nvstore.get(key, *size, user_buf, actual_size);
    fido_log_error("fido_flash_record_read: nvstore.get(%d) returns %d", key, ret);
    if (ret != NVSTORE_SUCCESS) {
        return false;
    }
    
    *size = actual_size;
    return true;
}

void fido_flash_init(void)
{
    NVStore &nvstore = NVStore::get_instance();
    int rc = nvstore.init();
    if (rc != NVSTORE_SUCCESS) {
        fido_log_error("fido_flash_init: nvstore.init returns %d", rc);
        return;
    }

    // for test
    uint32_t data = 12345678;
    size_t   size = sizeof(uint32_t);
    if (!fido_flash_record_write(NVSTORE_KEY_CONTEXT, &data, size)) {
        return;
    }
    if (!fido_flash_record_read(NVSTORE_KEY_CONTEXT, &data, &size)) {
        return;
    }
    fido_log_debug("fido_flash_record_read(%d bytes):", size);
    fido_log_print_hexdump_debug((uint8_t *)&data, size);
}
