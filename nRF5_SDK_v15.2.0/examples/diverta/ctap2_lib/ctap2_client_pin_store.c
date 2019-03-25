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

static bool pin_code_hash_record_find(fds_record_desc_t *record_desc)
{
    // 作業領域の初期化
    memset(m_pin_store_hash_record, 0, FIDO_PIN_STORE_HASH_RECORD_SIZE * 4);

    // Flash ROMから既存データを検索し、
    // 見つかった場合は true を戻す
    fds_find_token_t  ftok = {0};
    ret_code_t ret = fds_record_find(FIDO_PIN_STORE_FILE_ID, FIDO_PIN_STORE_HASH_RECORD_KEY, record_desc, &ftok);
    if (ret != FDS_SUCCESS) {
        return false;
    }

    // Flash ROMに登録されているデータを読み出す
    return fido_flash_fds_record_get(record_desc, m_pin_store_hash_record);
}

bool ctap2_client_pin_store_hash_read(void)
{
    // Flash ROMから既存データを読込み、
    // 既存データがあれば、データを
    // m_pin_store_hash_record に読込む
    fds_record_desc_t record_desc;
    return pin_code_hash_record_find(&record_desc);
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
    found = pin_code_hash_record_find(&record_desc);
    
    // PINコードハッシュ部 (8ワード)
    // NULLが引き渡された場合は、更新しないものとする
    if (p_pin_code_hash != NULL) {
        memcpy((uint8_t *)m_pin_store_hash_record, p_pin_code_hash, 32);
    }

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

bool ctap2_client_pin_store_pin_code_exist(void)
{
    // PINコードをFlash ROMから読み出し
    if (ctap2_client_pin_store_hash_read() == false) {
        return false;
    }

    // PINコードハッシュがゼロ埋めされている場合は未登録と判断
    uint8_t *pin_code_hash = ctap2_client_pin_store_pin_code_hash();
    uint8_t  pin_code_hash_size = FIDO_PIN_STORE_HASH_RECORD_SIZE * 4;
    uint8_t  i;
    for (i = 0; i < pin_code_hash_size; i++) {
        if (pin_code_hash[i] != 0) {
            return true;
        }
    }
    return false;
}
