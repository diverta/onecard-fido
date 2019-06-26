#include "sdk_common.h"

#include <stdio.h>
#include <string.h>

#include "fds.h"
#include "ble_u2f.h"
#include "ble_u2f_command.h"
#include "ble_u2f_status.h"

#include "u2f_authenticate.h"
#include "u2f_keyhandle.h"

#include "fido_ble_receive.h"
#include "fido_u2f_command.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

static uint8_t *get_appid_from_apdu(void)
{
    // appIdHashを参照し、APDUの33バイト目のアドレスを戻す
    uint8_t *p_appid_hash = fido_ble_receive_apdu()->data + U2F_CHAL_SIZE;

    return p_appid_hash;
}

void ble_u2f_authenticate_resume_process(void)
{
    // U2Fのリクエストデータを取得し、
    // レスポンス・メッセージを生成
    uint8_t *apdu_data = fido_ble_receive_apdu()->data;
    uint32_t apdu_le = fido_ble_receive_apdu()->Le;
    size_t  *u2f_response_length = fido_u2f_command_response_length();
    *u2f_response_length = fido_u2f_command_response_buffer_size();
    if (u2f_authenticate_response_message(apdu_data, fido_u2f_command_response_buffer(), u2f_response_length, apdu_le) == false) {
        // NGであれば、エラーレスポンスを生成して戻す
        uint8_t cmd = fido_ble_receive_header()->CMD;
        ble_u2f_send_error_response(cmd, fido_ble_receive_header()->STATUS_WORD);
        return;
    }
    
    // appIdHashをキーとして、
    // トークンカウンターレコードを更新
    // (fds_record_update/writeまたはfds_gcが実行される)
    uint8_t *p_appid_hash = get_appid_from_apdu();
    u2f_authenticate_update_token_counter(p_appid_hash);
}

void ble_u2f_authenticate_do_process(void)
{
    fido_log_debug("ble_u2f_authenticate start ");
    uint8_t cmd = fido_ble_receive_header()->CMD;

    if (fido_flash_skey_cert_read() == false) {
        // 秘密鍵と証明書をFlash ROMから読込
        // NGであれば、エラーレスポンスを生成して戻す
        ble_u2f_send_error_response(cmd, 0x9501);
        return;
    }

    uint8_t *apdu_data = fido_ble_receive_apdu()->data;
    if (u2f_authenticate_restore_keyhandle(apdu_data) == false) {
        // リクエストデータのキーハンドルを復号化し、
        // リクエストデータのappIDHashがキーハンドルに含まれていない場合、
        // エラーレスポンスを生成して戻す
        fido_log_error("ble_u2f_authenticate: invalid keyhandle ");
        ble_u2f_send_error_response(cmd, U2F_SW_WRONG_DATA);
        return;
    }

    // appIdHashをリクエストデータから取得し、
    // それに紐づくトークンカウンターを検索
    uint8_t *p_appid_hash = get_appid_from_apdu();
    if (fido_flash_token_counter_read(p_appid_hash) == false) {
        // appIdHashがトークンカウンターにない場合は
        // エラーレスポンスを生成して戻す
        fido_log_error("ble_u2f_authenticate: token counter not found ");
        ble_u2f_send_error_response(cmd, U2F_SW_WRONG_DATA);
        return;
    }
    fido_log_debug("U2F Authenticate: token counter value=%d ", fido_flash_token_counter_value());

    // control byte (P1) を参照
    uint8_t control_byte = fido_ble_receive_apdu()->P1;
    if (control_byte == 0x07) {
        // 0x07 ("check-only") の場合はここで終了し
        // SW_CONDITIONS_NOT_SATISFIED (0x6985)を戻す
        ble_u2f_send_error_response(cmd, U2F_SW_CONDITIONS_NOT_SATISFIED);
        return;
    }

    if (control_byte == 0x03) {
        // 0x03 ("enforce-user-presence-and-sign")
        // ユーザー所在確認が必要な場合は、ここで終了し
        // キープアライブ送信を開始する
        // ステータスバイトにTUP_NEEDED(0x02)を設定
        fido_log_info("ble_u2f_authenticate: waiting to complete the test of user presence");
        uint8_t keepalive_status_byte = 0x02;
        fido_user_presence_verify_start(U2F_KEEPALIVE_INTERVAL_MSEC, &keepalive_status_byte);
        return;
    }

    // ユーザー所在確認不要の場合は、ここで
    // ただちにレスポンス・メッセージを生成
    ble_u2f_authenticate_resume_process();
}

static void send_authentication_response(void)
{
    // レスポンスを生成
    uint8_t command_for_response = fido_ble_receive_header()->CMD;
    uint8_t *data_buffer = fido_u2f_command_response_buffer();
    uint16_t data_buffer_length = (uint16_t)(*fido_u2f_command_response_length());

    // 生成したレスポンスを戻す
    ble_u2f_status_setup(command_for_response, data_buffer, data_buffer_length);
    ble_u2f_status_response_send();
}

void ble_u2f_authenticate_send_response(fds_evt_t const *const p_evt)
{
    if (p_evt->result != FDS_SUCCESS) {
        // FDS処理でエラーが発生時は以降の処理を行わない
        uint8_t cmd = fido_ble_receive_header()->CMD;
        ble_u2f_send_error_response(cmd, 0x9503);
        fido_log_error("ble_u2f_authenticate abend: FDS EVENT=%d ", p_evt->id);
        return;
    }

    if (p_evt->id == FDS_EVT_GC) {
        // FDSリソース不足解消のためGCが実行された場合は、
        // GC実行直前の処理を再実行
        NRF_LOG_WARNING("ble_u2f_authenticate retry: FDS GC done ");
        uint8_t *p_appid_hash = get_appid_from_apdu();
        u2f_authenticate_update_token_counter(p_appid_hash);

    } else if (p_evt->id == FDS_EVT_UPDATE || p_evt->id == FDS_EVT_WRITE) {
        // レスポンスを生成してU2Fクライアントに戻す
        send_authentication_response();
        fido_log_debug("ble_u2f_authenticate end ");
    }
}
