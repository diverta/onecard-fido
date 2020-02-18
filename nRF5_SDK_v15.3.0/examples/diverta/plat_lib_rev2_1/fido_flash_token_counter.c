/* 
 * File:   fido_flash_token_counter.c
 * Author: makmorit
 *
 * Created on 2019/07/09, 10:22
 */
#include "sdk_common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fido_flash.h"
#include "fido_flash_common.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_flash_token_counter
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// Flash ROM書込み用データの一時格納領域
static uint32_t m_token_counter_record_buffer[FIDO_TOKEN_COUNTER_RECORD_SIZE];

bool fido_flash_token_counter_delete(void)
{
    // トークンカウンターをFlash ROM領域から削除
    ret_code_t err_code = fds_file_delete(FIDO_TOKEN_COUNTER_FILE_ID);
    if (err_code != FDS_SUCCESS) {
        NRF_LOG_ERROR("fds_file_delete returns 0x%02x ", err_code);
        return false;
    }

    return true;
}

static bool token_counter_record_find(uint8_t *p_unique_key, fds_record_desc_t *record_desc)
{
    ret_code_t ret;
    
    // Flash ROMから既存データを走査
    bool found = false;
    fds_find_token_t  ftok = {0};
    do {
        ret = fds_record_find(FIDO_TOKEN_COUNTER_FILE_ID, FIDO_TOKEN_COUNTER_RECORD_KEY, record_desc, &ftok);
        if (ret == FDS_SUCCESS) {
            // 同じキーのレコードかどうか判定 (先頭32バイトを比較)
            fido_flash_fds_record_get(record_desc, m_token_counter_record_buffer);
            if (strncmp((char *)p_unique_key, (char *)m_token_counter_record_buffer, 32) == 0) {
                found = true;
            }
        }
    } while (ret == FDS_SUCCESS && found == false);

    return found;
}

uint32_t fido_flash_token_counter_value(void)
{
    // カウンターを取得して戻す
    return m_token_counter_record_buffer[8];
}

uint8_t *fido_flash_token_counter_get_check_hash(void)
{
    // カウンターに紐づくチェック用ハッシュが
    // 格納されている先頭アドレスを戻す
    // バッファ先頭からのオフセットは９ワード分
    uint8_t *hash_for_check = (uint8_t *)(m_token_counter_record_buffer + 9);
    return hash_for_check;
}

uint8_t *fido_flash_token_counter_user_name(void)
{
    // ユーザー名が格納されている先頭アドレスを戻す
    // バッファ先頭からのオフセットは17ワード分
    return (uint8_t *)(m_token_counter_record_buffer + 17);
}

uint16_t fido_flash_token_counter_key_id(void)
{
    // 秘密鍵スロット番号を取得して戻す
    return (uint16_t)m_token_counter_record_buffer[25];
}

bool fido_flash_token_counter_read(uint8_t *p_unique_key)
{
    // Flash ROMから既存データを読込み、
    // 既存データがあれば、データを
    // m_token_counter_record_bufferに読込む
    fds_record_desc_t record_desc;
    return token_counter_record_find(p_unique_key, &record_desc);
}

bool fido_flash_token_counter_write(uint8_t *p_unique_key, uint32_t token_counter, uint8_t *p_rpid_hash, 
    uint8_t *p_username, size_t username_size, uint16_t key_id)
{
    // Flash ROMから既存データを走査
    bool found = false;
    fds_record_desc_t record_desc;
    found = token_counter_record_find(p_unique_key, &record_desc);
    
    // 新規登録の場合はデータ格納領域を初期化
    if (token_counter == 0) {
        memset((uint8_t *)m_token_counter_record_buffer, 0, FIDO_TOKEN_COUNTER_RECORD_SIZE * 4);
    }

    // ユニークキー部 (8ワード)
    memcpy((uint8_t *)m_token_counter_record_buffer, p_unique_key, 32);

    // トークンカウンター部 (1ワード)
    m_token_counter_record_buffer[8] = token_counter;

    // rpIdHash部 (8ワード)
    // バッファ先頭からのオフセットは９ワード（36バイト）分
    if (p_rpid_hash != NULL) {
        uint8_t *_rpid_hash = (uint8_t *)m_token_counter_record_buffer + 36;
        memcpy(_rpid_hash, p_rpid_hash, 32);
    }

    // username部 (8ワード) 
    // バッファ先頭からのオフセットは17ワード（68バイト）分
    // 最大31バイトセットできるものとする
    if (p_username != NULL) {
        size_t max_size = (username_size < 31) ? username_size : 31;
        uint8_t *_username = (uint8_t *)m_token_counter_record_buffer + 68;
        memcpy(_username, p_username, max_size);
    }

    // 秘密鍵スロット番号部 (1ワード)
    // 新規登録時のみ設定する
    if (token_counter == 0) {
        m_token_counter_record_buffer[25] = key_id;
    }

    // Flash ROMに書込むレコードを生成
    fds_record_t record;
    record.file_id           = FIDO_TOKEN_COUNTER_FILE_ID;
    record.key               = FIDO_TOKEN_COUNTER_RECORD_KEY;
    record.data.p_data       = m_token_counter_record_buffer;
    record.data.length_words = FIDO_TOKEN_COUNTER_RECORD_SIZE;

    ret_code_t ret;
    if (found == true) {
        // 既存のデータが存在する場合は上書き
        ret = fds_record_update(&record_desc, &record);
        if (ret != FDS_SUCCESS && ret != FDS_ERR_NO_SPACE_IN_FLASH) {
            NRF_LOG_ERROR("fds_record_update returns 0x%02x ", ret);
            return false;
        }

    } else {
        // 既存のデータが存在しない場合は新規追加
        ret = fds_record_write(&record_desc, &record);
        if (ret != FDS_SUCCESS && ret != FDS_ERR_NO_SPACE_IN_FLASH) {
            NRF_LOG_ERROR("fds_record_write returns 0x%02x ", ret);
            return false;
        }
    }

    if (ret == FDS_ERR_NO_SPACE_IN_FLASH) {
        // 書込みができない場合、ガベージコレクションを実行
        // (fds_gcが実行される。NGであればシステムエラー扱い)
        NRF_LOG_ERROR("no space in flash, calling FDS GC ");
        fido_flash_fds_force_gc();
    }

    return true;
}
