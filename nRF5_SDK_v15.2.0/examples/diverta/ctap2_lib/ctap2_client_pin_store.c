/* 
 * File:   ctap2_client_pin_store.c
 * Author: makmorit
 *
 * Created on 2019/02/27, 10:43
 */
#include "sdk_common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fido_flash.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ctap2_client_pin_store
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// Flash ROM書込み用データの一時格納領域
// 
// PINコードハッシュ管理用
//   レコードサイズ = 9 ワード
//     PINコードハッシュ: 8ワード（32バイト）
//     リトライカウンター: 1ワード（4バイト)
static uint32_t m_pin_store_hash_record[FIDO_PIN_STORE_HASH_RECORD_SIZE];

static bool pin_code_hash_record_find(uint8_t *p_pin_code_hash, fds_record_desc_t *record_desc)
{
    ret_code_t ret;
    
    // Flash ROMから既存データを走査
    bool found = false;
    fds_find_token_t  ftok = {0};
    do {
        ret = fds_record_find(FIDO_PIN_STORE_FILE_ID, FIDO_PIN_STORE_HASH_RECORD_KEY, record_desc, &ftok);
        if (ret == FDS_SUCCESS) {
            // 同じPINコードハッシュのレコードかどうか判定
            // (先頭16バイトだけを比較)
            fido_flash_fds_record_get(record_desc, m_pin_store_hash_record);
            if (strncmp((char *)p_pin_code_hash, (char *)m_pin_store_hash_record, 16) == 0) {
                found = true;
            }
        }
    } while (ret == FDS_SUCCESS && found == false);

    return found;
}

bool ctap2_client_pin_store_hash_read(uint8_t *p_pin_code_hash)
{
    // Flash ROMから既存データを読込み、
    // 既存データがあれば、データを
    // m_pin_store_hash_record に読込む
    fds_record_desc_t record_desc;
    return pin_code_hash_record_find(p_pin_code_hash, &record_desc);
}

uint8_t *ctap2_client_pin_store_pin_code_hash(void)
{
    // レコード領域の先頭アドレスを戻す
    return (uint8_t *)m_pin_store_hash_record;
}

uint32_t ctap2_client_pin_store_retry_counter(void)
{
    // カウンターを取得して戻す
    // （レコード領域先頭から９ワード目）
    return m_pin_store_hash_record[8];
}

bool ctap2_client_pin_store_hash_write(uint8_t *p_pin_code_hash, uint32_t retry_counter)
{
    // Flash ROMから既存データを走査
    bool found = false;
    fds_record_desc_t record_desc;
    found = pin_code_hash_record_find(p_pin_code_hash, &record_desc);
    
    // ユニークキーとなるPINコードハッシュ部 (8ワード)
    memcpy((uint8_t *)m_pin_store_hash_record, p_pin_code_hash, 32);

    // トークンカウンター部 (1ワード)
    m_pin_store_hash_record[8] = retry_counter;

    // Flash ROMに書込むレコードを生成
    fds_record_t record;
    record.file_id           = FIDO_PIN_STORE_FILE_ID;
    record.key               = FIDO_PIN_STORE_HASH_RECORD_KEY;
    record.data.p_data       = m_pin_store_hash_record;
    record.data.length_words = FIDO_PIN_STORE_HASH_RECORD_SIZE;

    ret_code_t ret;
    if (found == true) {
        // 既存のデータが存在する場合は上書き
        ret = fds_record_update(&record_desc, &record);
        if (ret != FDS_SUCCESS && ret != FDS_ERR_NO_SPACE_IN_FLASH) {
            NRF_LOG_ERROR("ctap2_client_pin_store_hash_write: fds_record_update returns 0x%02x ", ret);
            return false;
        }

    } else {
        // 既存のデータが存在しない場合は新規追加
        ret = fds_record_write(&record_desc, &record);
        if (ret != FDS_SUCCESS && ret != FDS_ERR_NO_SPACE_IN_FLASH) {
            NRF_LOG_ERROR("ctap2_client_pin_store_hash_write: fds_record_write returns 0x%02x ", ret);
            return false;
        }
    }

    if (ret == FDS_ERR_NO_SPACE_IN_FLASH) {
        // 書込みができない場合、ガベージコレクションを実行
        // (fds_gcが実行される。NGであればエラー扱い)
        NRF_LOG_ERROR("ctap2_client_pin_store_hash_write: no space in flash, calling FDS GC ");
        if (fido_flash_force_fdc_gc() == false) {
            return false;
        }
    }

    return true;
}