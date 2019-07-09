/* 
 * File:   fido_flash_event.c
 * Author: makmorit
 *
 * Created on 2019/06/19, 10:10
 */
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

void fido_flash_event_fds_register(void)
{
    // FDS処理完了後の処理をFDSに登録
    ret_code_t err_code = fds_register(fido_command_on_fs_evt);
    APP_ERROR_CHECK(err_code);
}

void fido_flash_event_gc_forced(void)
{
    // アプリケーション側でGCを発生させた旨のフラグを設定
    m_gc_forced = true;
}
