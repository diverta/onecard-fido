/* 
 * File:   fido_flash_common.c
 * Author: makmorit
 *
 * Created on 2019/06/19, 10:10
 */
#include "sdk_common.h"
#include "app_error.h"
#include "sdk_config.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_flash_common
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

//
// プラットフォーム依存コード
// ターゲットごとの実装となります。
//
#include "fido_flash.h"
#include "fido_flash_common.h"

//
// 共通関数
//
void fido_flash_fds_force_gc(void)
{
    // FDSガベージコレクションを強制実行
    // NGの場合はシステムエラー扱い（処理続行不可）
    ret_code_t err_code = fds_gc();
    if (err_code != FDS_SUCCESS) {
        APP_ERROR_CHECK(err_code);
    }

    // アプリケーション側でGCを発生させた旨のフラグを設定
    fido_flash_event_set_gc_forced();
}

bool fido_flash_fds_record_get(fds_record_desc_t *record_desc, uint32_t *record_buffer)
{
    fds_flash_record_t flash_record;
    uint32_t  *data;
    uint16_t   data_length;
    ret_code_t err_code;

    err_code = fds_record_open(record_desc, &flash_record);
    if (err_code != FDS_SUCCESS) {
        NRF_LOG_ERROR("fds_record_open returns 0x%02x ", err_code);
        return false;
    }

    data = (uint32_t *)flash_record.p_data;
    data_length = flash_record.p_header->length_words;
    memcpy(record_buffer, data, data_length * sizeof(uint32_t));

    err_code = fds_record_close(record_desc);
    if (err_code != FDS_SUCCESS) {
        NRF_LOG_ERROR("fds_record_close returns 0x%02x ", err_code);
        return false;	
    }

    return true;
}

static bool fido_flash_fds_record_find(uint16_t file_id, uint16_t record_key, size_t record_words, uint32_t *record_buf_R, fds_record_desc_t *record_desc, bool *is_exist)
{
    // 作業領域の初期化
    memset(record_desc, 0, sizeof(fds_record_desc_t));
    memset(record_buf_R, 0, record_words * 4);

    // Flash ROMから既存データを検索し、
    fds_find_token_t ftok = {0};
    ret_code_t ret = fds_record_find(file_id, record_key, record_desc, &ftok);
    if (ret == FDS_SUCCESS) {
        // 見つかった場合は is_exist に true を設定し、
        // Flash ROMに登録されているデータを読み出す
        *is_exist = true;
        return fido_flash_fds_record_get(record_desc, record_buf_R);

    } else if (ret == FDS_ERR_NOT_FOUND) {
        // 見つからなかった場合は is_exist に false を設定
        *is_exist = false;
        return true;

    } else {
        // 処理失敗時
        return false;
    }
}

bool fido_flash_fds_record_read(uint16_t file_id, uint16_t record_key, size_t record_words, uint32_t *record_buf_R, bool *is_exist)
{
    // Flash ROMから既存データを走査
    fds_record_desc_t record_desc;
    return fido_flash_fds_record_find(file_id, record_key, record_words, record_buf_R, &record_desc, is_exist);
}

bool fido_flash_fds_record_write(uint16_t file_id, uint16_t record_key, size_t record_words, uint32_t *record_buf_R, uint32_t *record_buf_W)
{
    // Flash ROMから既存データを走査
    fds_record_desc_t record_desc;
    bool found;
    if (fido_flash_fds_record_find(file_id, record_key, record_words, record_buf_R, &record_desc, &found) == false) {
        return false;
    }

    // Flash ROMに書込むレコードを生成
    fds_record_t record;
    record.file_id           = file_id;
    record.key               = record_key;
    record.data.p_data       = record_buf_W;
    record.data.length_words = record_words;

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

//
// 業務処理／HW依存処理間のインターフェース
//
bool fido_flash_get_stat_csv(uint8_t *stat_csv_data, size_t *stat_csv_size)
{
    // 格納領域を初期化
    memset(stat_csv_data, 0, *stat_csv_size);

    // nRF5 SDK経由でFlash ROM統計情報を取得
    fds_stat_t stat = {0};
    ret_code_t ret = fds_stat(&stat);
    if (ret != FDS_SUCCESS) {
        NRF_LOG_ERROR("fds_stat returns 0x%02x ", ret);
        return false;
    }

    // 各項目をCSV化し、引数のバッファに格納
    sprintf((char *)stat_csv_data, 
        "words_available=%d,words_used=%d,freeable_words=%d,largest_contig=%d,valid_records=%d,dirty_records=%d,corruption=%d", 
        (stat.pages_available - 1) * FDS_VIRTUAL_PAGE_SIZE, 
        stat.words_used, 
        stat.freeable_words,
        stat.largest_contig,
        stat.valid_records, 
        stat.dirty_records,
        stat.corruption);
    *stat_csv_size = strlen((char *)stat_csv_data);
    NRF_LOG_DEBUG("Flash ROM statistics csv created (%d bytes)", *stat_csv_size);
    return true;
}
