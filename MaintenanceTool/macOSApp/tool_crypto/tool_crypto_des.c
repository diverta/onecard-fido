//
//  tool_crypto_des.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/11/27.
//
#include <string.h>

#include "debug_log.h"
#include "tool_crypto_des.h"

// for OpenSSL
#include <openssl/des.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

// デフォルトの3DES鍵
static const unsigned char default_3des_key[] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
};

// DES鍵を保持
typedef struct _des_key {
    DES_key_schedule ks1;
    DES_key_schedule ks2;
    DES_key_schedule ks3;
} des_key;
static des_key *mgm_key = NULL;

static void des_destroy_key(des_key *key) {
    if (key != NULL) {
        free(key);
    }
}

static bool des_import_key(const int type, const unsigned char *key_raw, const size_t key_raw_size)
{
    // 変数初期化
    const_DES_cblock key_tmp;
    size_t key_tmp_size = sizeof(key_tmp);
    // パラメーターチェック
    if (!key_raw) {
        log_debug("%s: DES parameter is NULL", __func__);
        return false;
    }
    if (key_raw_size != DES_LEN_3DES) {
        log_debug("%s: DES invalid key size", __func__);
        return false;
    }
    if ((mgm_key = (des_key *)malloc(sizeof(des_key))) == NULL) {
        log_debug("%s: DES memory allocation error", __func__);
        return false;
    }
    // 3DESキーの内容をバッファにコピー
    memcpy(key_tmp, key_raw, key_tmp_size);
    DES_set_key_unchecked(&key_tmp, &(mgm_key->ks1));
    memcpy(key_tmp, key_raw + key_tmp_size, key_tmp_size);
    DES_set_key_unchecked(&key_tmp, &(mgm_key->ks2));
    memcpy(key_tmp, key_raw + (2 * key_tmp_size), key_tmp_size);
    DES_set_key_unchecked(&key_tmp, &(mgm_key->ks3));
    return true;
}

//
// public functions
//
unsigned char *tool_crypto_des_default_key(void)
{
    return (unsigned char *)default_3des_key;
}

bool tool_crypto_des_import_key(const unsigned char *keyraw, const size_t keyrawlen)
{
    if (mgm_key != NULL) {
        des_destroy_key(mgm_key);
    }
    return des_import_key(DES_TYPE_3DES, keyraw, keyrawlen);
}
