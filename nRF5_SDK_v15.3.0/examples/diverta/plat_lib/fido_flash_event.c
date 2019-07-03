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
// FDSから渡されたイベント情報を保持
//
static fido_flash_event_t flash_event;

static void fido_command_send_response(void *const p_evt)
{
    // BLEペアリングコマンドの処理を実行
    fido_ble_pairing_reflect_mode_change(p_evt);

    // U2Fコマンドの処理を実行
    fido_u2f_command_msg_send_response(p_evt);

    // CTAP2コマンドの処理を実行
    fido_ctap2_command_cbor_send_response(p_evt);

    // 管理用コマンドの処理を実行
    fido_maintenance_command_send_response(p_evt);
}

static void fido_command_on_fs_evt(fds_evt_t const *p_evt)
{
    // 処理結果を構造体に保持
    flash_event.result = (p_evt->result == FDS_SUCCESS);
    flash_event.gc = (p_evt->id == FDS_EVT_GC);
    flash_event.delete_file = (p_evt->id == FDS_EVT_DEL_FILE);
    flash_event.write_update = (p_evt->id == FDS_EVT_UPDATE || p_evt->id == FDS_EVT_WRITE);
    flash_event.retry_counter_write = (p_evt->write.record_key == FIDO_PIN_RETRY_COUNTER_RECORD_KEY);
    flash_event.token_counter_write = (p_evt->write.record_key == FIDO_TOKEN_COUNTER_RECORD_KEY);
    flash_event.skey_cert_write = (p_evt->write.record_key == FIDO_SKEY_CERT_RECORD_KEY);
    flash_event.aeskeys_write = (p_evt->write.record_key == FIDO_AESKEYS_RECORD_KEY);
    flash_event.pairing_mode_write = (p_evt->write.record_key == FIDO_PAIRING_MODE_RECORD_KEY);

    // FDS処理完了後の業務レスポンス処理を実行
    fido_command_send_response(&flash_event);
}

void fido_command_fds_register(void)
{
    // FDS処理完了後の処理をFDSに登録
    ret_code_t err_code = fds_register(fido_command_on_fs_evt);
    APP_ERROR_CHECK(err_code);
}
