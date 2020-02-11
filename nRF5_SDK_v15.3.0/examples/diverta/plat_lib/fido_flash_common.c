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
	uint32_t *data;
    uint16_t  data_length;
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
