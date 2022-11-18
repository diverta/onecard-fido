/* 
 * File:   ccid_flash_oath_object.c
 * Author: makmorit
 *
 * Created on 2022/11/15, 17:44
 */
#include "sdk_common.h"

#include "fido_flash_plat.h"
#include "fido_flash_common.h"

#include "ccid_flash_object.h"
#include "ccid_oath.h"
#include "ccid_oath_define.h"
#include "ccid_oath_object.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ccid_flash_oath_object
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// バッファサイズの最大長
#define MAX_BUF_SIZE     (OATH_DATA_OBJ_WORDS_MAX+1)

//
// FDSによりアクセスした物理レコード情報を保持
//
static fds_record_desc_t m_fds_record_desc;

// レコード走査（fetch）時に実行する関数
static bool fetch_records(uint16_t file_id, uint16_t record_key, uint8_t *p_unique_key, size_t unique_key_size, size_t unique_key_offset, bool (*function)(fds_record_desc_t *record_desc), bool *exist);

// Flash ROM書込み時に実行した関数の参照を保持
static void *m_flash_func = NULL;

//
// オブジェクトのRead／Write
//
static bool get_record_key_by_tag(uint8_t tag, uint16_t *record_key)
{
    switch (tag) {
        default:
            *record_key = OATH_DATA_OBJ_01_RECORD_KEY;
            break;
    }
    return true;
}

static bool fetch_record_for_update(uint16_t file_id, uint16_t record_key, size_t record_words, uint8_t *record_bytes)
{
    // record_bytesから、該当レコードのユニークキーを取得
    size_t unique_key_size = 64;
    size_t unique_key_offset = OATH_DATA_OBJ_ATTR_WORDS * 4;
    uint8_t *p_unique_key = record_bytes + unique_key_offset;

    // Flash ROMから既存データを走査
    bool found;
    if (fetch_records(file_id, record_key, p_unique_key, unique_key_size, unique_key_offset, NULL, &found) == false) {
        return false;
    }

    // Flash ROMに書込むレコードを生成
    fds_record_t record;
    record.file_id           = file_id;
    record.key               = record_key;
    record.data.p_data       = (uint32_t *)record_bytes;
    record.data.length_words = record_words;

    ret_code_t ret;
    if (found) {
        // 既存のデータが存在する場合は上書き
        ret = fds_record_update(&m_fds_record_desc, &record);
        if (ret != NRF_SUCCESS && ret != FDS_ERR_NO_SPACE_IN_FLASH) {
            NRF_LOG_ERROR("fetch_record_for_update: fds_record_update returns 0x%02x ", ret);
            return false;
        }

    } else {
        // 既存のデータが存在しない場合は新規追加
        fds_record_desc_t record_desc;
        ret = fds_record_write(&record_desc, &record);
        if (ret != NRF_SUCCESS && ret != FDS_ERR_NO_SPACE_IN_FLASH) {
            NRF_LOG_ERROR("fetch_record_for_update: fds_record_write returns 0x%02x ", ret);
            return false;
        }
    }

    if (ret == FDS_ERR_NO_SPACE_IN_FLASH) {
        // 書込みができない場合、ガベージコレクションを実行
        // (fds_gcが実行される。NGであればシステムエラー扱い)
        NRF_LOG_ERROR("fetch_record_for_update: no space in flash, calling FDS GC ");
        fido_flash_fds_force_gc();
    }

    return true;
}

bool ccid_flash_oath_object_write(uint16_t obj_tag, uint8_t *obj_buff, size_t obj_size, bool use_serial, uint16_t serial)
{
    // ファイル名を取得
    uint16_t file_id = OATH_DATA_OBJ_FILE_ID;

    // レコードキーを取得
    uint16_t record_key;
    if (get_record_key_by_tag(obj_tag, &record_key) == false) {
        return false;
    }
    
    // 引数のデータを、Flash ROM書込み用データの一時格納領域にコピー
    //   オブジェクトデータの長さ: 1ワード（4バイト）
    //   オブジェクトデータ = 可変長（最大256ワード＝1,024バイト）
    uint8_t *rec_bytes = ccid_flash_object_write_buffer();
    uint32_t size32_t = (uint32_t)obj_size;
    memcpy(rec_bytes, &size32_t, sizeof(uint32_t));
    memcpy(rec_bytes + 4, obj_buff, obj_size);

    // オブジェクトデータの長さから、必要ワード数を計算
    size_t record_words = ccid_flash_object_calculate_words(size32_t) + OATH_DATA_OBJ_ATTR_WORDS;

    // データをFlash ROMに書込
    m_flash_func = (void *)ccid_flash_oath_object_write;
    return fetch_record_for_update(file_id, record_key, record_words, rec_bytes);
}

static bool fetch_records(uint16_t file_id, uint16_t record_key, uint8_t *p_unique_key, size_t unique_key_size, size_t unique_key_offset, bool (*function)(fds_record_desc_t *record_desc), bool *exist)
{
    ret_code_t ret;
    uint32_t *read_buffer = (uint32_t *)ccid_flash_object_read_buffer();
    
    // Flash ROMから既存データを走査
    bool success = true;
    bool found = false;
    fds_find_token_t ftok = {0};
    do {
        ret = fds_record_find(file_id, record_key, &m_fds_record_desc, &ftok);
        if (ret == NRF_SUCCESS) {
            // 同じキーのレコードかどうか判定 (先頭バイトを比較)
            fido_flash_fds_record_get(&m_fds_record_desc, read_buffer);
            uint8_t *p_read_buffer = (uint8_t *)read_buffer;
            if (memcmp(p_unique_key, p_read_buffer + unique_key_offset, unique_key_size) == 0) {
                found = true;
                if (function != NULL) {
                    success = function(&m_fds_record_desc);
                }
                break;
            }
        }
    } while (ret == NRF_SUCCESS && found == false);

    *exist = found;
    return success;
}

bool ccid_flash_oath_object_find(uint16_t obj_tag, uint8_t *p_unique_key, size_t unique_key_size, uint8_t *p_record_buffer, bool *exist, uint16_t *serial)
{
    // ファイル名を取得
    uint16_t file_id = OATH_DATA_OBJ_FILE_ID;

    // レコードキーを取得
    uint16_t record_key;
    if (get_record_key_by_tag(obj_tag, &record_key) == false) {
        return false;
    }

    // Flash ROMから属性データ（オブジェクト長＝１ワード）を読込
    // 既存データがなければ、最初の連番をセットして終了
    size_t unique_key_offset = OATH_DATA_OBJ_ATTR_WORDS * 4;
    fetch_records(file_id, record_key, p_unique_key, unique_key_size, unique_key_offset, NULL, exist);
    if (*exist == false) {
        *serial = 0;
        return true;
    } else {
        *serial = 1;
    }

    // 属性データを取出し、一時変数に保持
    //   オブジェクト属性 = 1ワード
    //     オブジェクトデータの長さ: 1ワード（4バイト）
    uint8_t *rec_bytes = ccid_flash_object_read_buffer();
    uint32_t size32_t;
    memcpy(&size32_t, rec_bytes, sizeof(uint32_t));

#if HEXDUMP_DEBUG_OBJECT_FOUND
    size_t total_size = size32_t + 4;
    NRF_LOG_DEBUG("ccid_flash_oath_object_find (%d bytes)", total_size);
    NRF_LOG_HEXDUMP_DEBUG(rec_bytes, 32);
    NRF_LOG_DEBUG("last 16 bytes:");
    NRF_LOG_HEXDUMP_DEBUG(rec_bytes + total_size - 16, 16);
#endif

    // オブジェクトデータの長さから、必要ワード数を計算
    size_t record_words = ccid_flash_object_calculate_words(size32_t) + OATH_DATA_OBJ_ATTR_WORDS;

    // Flash ROMからオブジェクトデータを読込
    //   データが存在する場合は、
    //   m_record_buf_Rの２ワード目を先頭とし、
    //   オブジェクトデータが格納されます
    //   オブジェクトデータ = 可変長（最大256ワード＝1,024バイト）
    uint32_t *read_buffer = (uint32_t *)ccid_flash_object_read_buffer();
    return fido_flash_fds_record_read(file_id, record_key, record_words, read_buffer, exist);
}

static bool delete_record(fds_record_desc_t *record_desc)
{
    // レコードをFlash ROMから削除
    ret_code_t ret = fds_record_delete(record_desc);
    return (ret == NRF_SUCCESS);
}

bool ccid_flash_oath_object_delete(uint16_t obj_tag, uint8_t *p_unique_key, size_t unique_key_size, uint8_t *p_record_buffer, bool *exist, uint16_t *serial)
{
    // ファイル名を取得
    uint16_t file_id = OATH_DATA_OBJ_FILE_ID;

    // レコードキーを取得
    uint16_t record_key;
    if (get_record_key_by_tag(obj_tag, &record_key) == false) {
        return false;
    }

    // Flash ROMから既存データを削除
    m_flash_func = (void *)ccid_flash_oath_object_delete;
    size_t unique_key_offset = OATH_DATA_OBJ_ATTR_WORDS * 4;
    if (fetch_records(file_id, record_key, p_unique_key, unique_key_size, unique_key_offset, delete_record, exist) == false) {
        return false;
    }
    
    return true;
}

bool ccid_flash_oath_object_delete_all(void)
{
    // ファイル名を取得
    uint16_t file_id = OATH_DATA_OBJ_FILE_ID;

    // データファイルをFlash ROMから削除
    m_flash_func = (void *)ccid_flash_oath_object_delete_all;
    return fido_flash_fds_file_delete(file_id);
}

bool ccid_flash_oath_object_fetch(uint16_t obj_tag, int (*_fetch_func)(const char *key, void *data, size_t size))
{
    // ファイル名を取得
    uint16_t file_id = OATH_DATA_OBJ_FILE_ID;

    // レコードキーを取得
    uint16_t record_key;
    if (get_record_key_by_tag(obj_tag, &record_key) == false) {
        return false;
    }

    // Flash ROMから既存データを走査
    ret_code_t ret;
    uint32_t *read_buffer = (uint32_t *)ccid_flash_object_read_buffer();
    fds_find_token_t ftok = {0};
    do {
        ret = fds_record_find(file_id, record_key, &m_fds_record_desc, &ftok);
        if (ret == NRF_SUCCESS) {
            // レコードを取得
            fido_flash_fds_record_get(&m_fds_record_desc, read_buffer);
            uint8_t *p_read_buffer = (uint8_t *)read_buffer;

            // レコード長を先頭４バイトから取得
            uint32_t size32_t;
            memcpy(&size32_t, p_read_buffer, sizeof(uint32_t));

            // コールバック関数を実行
            if ((*_fetch_func)("", p_read_buffer, size32_t) != 0) {
                return false;
            }
        }
    } while (ret == NRF_SUCCESS);

    return true;
}

//
// コールバック関数群
//
void ccid_flash_oath_object_failed(void)
{
    if (m_flash_func == NULL) {
        return;
    }

    // 判定用の参照を初期化
    void *flash_func = m_flash_func;
    m_flash_func = NULL;

    // Flash ROM処理でエラーが発生時はエラーレスポンス送信
    if (flash_func == ccid_flash_oath_object_write ||
        flash_func == ccid_flash_oath_object_delete ||
        flash_func == ccid_flash_oath_object_delete_all) {
        ccid_oath_object_write_resume(false);
    }
}

void ccid_flash_oath_object_gc_done(void)
{
    if (m_flash_func == NULL) {
        return;
    }

    // 判定用の参照を初期化
    void *flash_func = m_flash_func;
    m_flash_func = NULL;

    // for nRF52840:
    // FDSリソース不足解消のためGCが実行された場合は、
    // GC実行直前の処理を再実行
    if (flash_func == ccid_flash_oath_object_write ||
        flash_func == ccid_flash_oath_object_delete ||
        flash_func == ccid_flash_oath_object_delete_all) {
        ccid_oath_object_write_retry();
    }
}

void ccid_flash_oath_object_record_updated(void)
{
    if (m_flash_func == NULL) {
        return;
    }

    // 判定用の参照を初期化
    void *flash_func = m_flash_func;
    m_flash_func = NULL;

    // 正常系の後続処理を実行
    if (flash_func == ccid_flash_oath_object_write) {
        ccid_oath_object_write_resume(true);
    }
}

void ccid_flash_oath_object_record_deleted(void)
{
    if (m_flash_func == NULL) {
        return;
    }

    // 判定用の参照を初期化
    void *flash_func = m_flash_func;
    m_flash_func = NULL;

    // 正常系の後続処理を実行
    if (flash_func == ccid_flash_oath_object_delete) {
        ccid_oath_object_write_resume(true);
    }
}

void ccid_flash_oath_object_file_deleted(void)
{
    if (m_flash_func == NULL) {
        return;
    }

    // 判定用の参照を初期化
    void *flash_func = m_flash_func;
    m_flash_func = NULL;

    // 正常系の後続処理を実行
    if (flash_func == ccid_flash_oath_object_delete_all) {
        ccid_oath_object_write_resume(true);
    }
}
