/* 
 * File:   fido_flash_common.c
 * Author: makmorit
 *
 * Created on 2019/06/19, 10:10
 */
#include "sdk_common.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_flash_common
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

//
// プラットフォーム非依存コード
//
#include "fido_ctap2_command.h"
#include "fido_maintenance.h"
#include "fido_u2f_command.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// プラットフォーム依存コード
// ターゲットごとの実装となります。
//
#include "fido_ble_pairing.h"
#include "fido_flash.h"

//
// GCがアプリケーション（業務処理）から
// 起動されたかどうかを保持するフラグ
//
static bool m_gc_forced;

//
// イベント管理
//
static void fido_flash_event_result_failure(void)
{
    // BLEペアリングコマンドの処理を実行
    fido_ble_pairing_flash_failed();

    // U2Fコマンドの処理を実行
    fido_u2f_command_flash_failed();

    // CTAP2コマンドの処理を実行
    fido_ctap2_command_flash_failed();

    // 管理用コマンドの処理を実行
    fido_maintenance_command_flash_failed();
}

static void fido_flash_event_gc_done(void)
{
    // BLEペアリングコマンドの処理を実行
    fido_ble_pairing_flash_gc_done();

    // U2Fコマンドの処理を実行
    fido_u2f_command_flash_gc_done();
    
    // CTAP2コマンドの処理を実行
    fido_ctap2_command_flash_gc_done();
    
    // 管理用コマンドの処理を実行
    fido_maintenance_command_flash_gc_done();
}

static void fido_flash_event_updated(fds_evt_t const *p_evt)
{
    // ペアリングモードレコードの書込み成功時
    if (p_evt->write.record_key == FIDO_PAIRING_MODE_RECORD_KEY) {
        // BLEペアリングコマンドの処理を実行
        fido_ble_pairing_flash_updated();

    } else if (p_evt->write.record_key == FIDO_AESKEYS_RECORD_KEY) {
        // 管理用コマンドの処理を実行
        fido_maintenance_command_aes_password_record_updated();

    } else if (p_evt->write.record_key == FIDO_SKEY_CERT_RECORD_KEY) {
        // 管理用コマンドの処理を実行
        fido_maintenance_command_skey_cert_record_updated();

    } else if (p_evt->write.record_key == FIDO_PIN_RETRY_COUNTER_RECORD_KEY) {
        // CTAP2コマンドの処理を実行
        fido_ctap2_command_retry_counter_record_updated();

    } else if (p_evt->write.record_key == FIDO_TOKEN_COUNTER_RECORD_KEY) {
        // U2Fコマンドの処理を実行
        fido_u2f_command_token_counter_record_updated();

        // CTAP2コマンドの処理を実行
        fido_ctap2_command_token_counter_record_updated();
    }
 }

static void fido_flash_event_file_deleted(fds_evt_t const *p_evt)
{
    if (p_evt->del.file_id == FIDO_SKEY_CERT_FILE_ID) {
        // 管理用コマンドの処理を実行
        fido_maintenance_command_skey_cert_file_deleted();

    } else if (p_evt->del.file_id == FIDO_TOKEN_COUNTER_FILE_ID) {
        // CTAP2コマンドの処理を実行
        fido_ctap2_command_token_counter_file_deleted();
        // 管理用コマンドの処理を実行
        fido_maintenance_command_token_counter_file_deleted();
    }
}

static void fido_command_on_fs_evt(fds_evt_t const *p_evt)
{
    // FDS処理が失敗時
    if (p_evt->result != FDS_SUCCESS) {
        fido_flash_event_result_failure();
        return;
    }

    // GCの場合は、業務側が発生させたものであれば、イベントを通知するようにする
    if (m_gc_forced) {
        m_gc_forced = false;
        if (p_evt->id == FDS_EVT_GC) {
            fido_flash_event_gc_done();
            return;
        }
    }
    
    if (p_evt->id == FDS_EVT_UPDATE || p_evt->id == FDS_EVT_WRITE) {
        // レコード書込時
        fido_flash_event_updated(p_evt);

    } else if (p_evt->id == FDS_EVT_DEL_FILE) {
        // ファイル削除時
        fido_flash_event_file_deleted(p_evt);
    }
}

void fido_flash_fds_event_register(void)
{
    // FDS処理完了後の処理をFDSに登録
    ret_code_t err_code = fds_register(fido_command_on_fs_evt);
    APP_ERROR_CHECK(err_code);
}

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
    m_gc_forced = true;
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
