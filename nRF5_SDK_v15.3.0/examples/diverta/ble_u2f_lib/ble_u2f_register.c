#include "sdk_common.h"

#include <stdio.h>
#include <string.h>

#include "fds.h"
#include "ble_u2f.h"
#include "ble_u2f_command.h"
#include "ble_u2f_status.h"

#include "u2f_keyhandle.h"
#include "u2f_register.h"

#include "fido_u2f_command.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

void ble_u2f_register_do_process(void)
{
    fido_log_debug("ble_u2f_register start ");
    ble_u2f_context_t *p_u2f_context = get_ble_u2f_context();
    uint8_t cmd = p_u2f_context->p_ble_header->CMD;

    if (fido_flash_skey_cert_read() == false) {
        // 秘密鍵と証明書をFlash ROMから読込
        // NGであれば、エラーレスポンスを生成して戻す
        ble_u2f_send_error_response(cmd, 0x9401);
        return;
    }

    if (fido_flash_skey_cert_available() == false) {
        // 秘密鍵と証明書がFlash ROMに登録されていない場合
        // エラーレスポンスを生成して戻す
        ble_u2f_send_error_response(cmd, 0x9402);
        return;
    }
    
    // APDUから取得したappIdHash、秘密鍵を使用し、
    // キーハンドルを新規生成
    uint8_t *p_appid_hash = p_u2f_context->p_apdu->data + U2F_CHAL_SIZE;
    u2f_register_generate_keyhandle(p_appid_hash);

    uint8_t *apdu_data = p_u2f_context->p_apdu->data;
    uint32_t apdu_le = p_u2f_context->p_apdu->Le;
    size_t  *u2f_response_length = fido_u2f_command_response_length();
    *u2f_response_length = fido_u2f_command_response_buffer_size();
    if (u2f_register_response_message(apdu_data, fido_u2f_command_response_buffer(), u2f_response_length, apdu_le) == false) {
        // U2Fのリクエストデータを取得し、
        // レスポンス・メッセージを生成
        // NGであれば、エラーレスポンスを生成して戻す
        ble_u2f_send_error_response(cmd, p_u2f_context->p_ble_header->STATUS_WORD);
        return;
    }

    // トークンカウンターレコードを追加
    // (fds_record_update/writeまたはfds_gcが実行される)
    u2f_register_add_token_counter(p_appid_hash);
}

static void send_register_response(ble_u2f_context_t *p_u2f_context)
{
    // レスポンスを生成
    uint8_t command_for_response = p_u2f_context->p_ble_header->CMD;
    uint8_t *data_buffer = fido_u2f_command_response_buffer();
    uint16_t data_buffer_length = (uint16_t)(*fido_u2f_command_response_length());

    // 生成したレスポンスを戻す
    ble_u2f_status_setup(command_for_response, data_buffer, data_buffer_length);
    ble_u2f_status_response_send();
}

void ble_u2f_register_send_response(fds_evt_t const *const p_evt)
{
    ble_u2f_context_t *p_u2f_context = get_ble_u2f_context();
    if (p_evt->result != FDS_SUCCESS) {
        // FDS処理でエラーが発生時は以降の処理を行わない
        uint8_t cmd = p_u2f_context->p_ble_header->CMD;
        ble_u2f_send_error_response(cmd, 0x9404);
        fido_log_error("ble_u2f_register abend: FDS EVENT=%d ", p_evt->id);
        return;
    }

    if (p_evt->id == FDS_EVT_GC) {
        // FDSリソース不足解消のためGCが実行された場合は、
        // GC実行直前の処理を再実行
        fido_log_warning("ble_u2f_register retry: FDS GC done ");
        uint8_t *p_appid_hash = p_u2f_context->p_apdu->data + U2F_CHAL_SIZE;
        u2f_register_add_token_counter(p_appid_hash);

    } else if (p_evt->id == FDS_EVT_UPDATE || p_evt->id == FDS_EVT_WRITE) {
        // レスポンスを生成してU2Fクライアントに戻す
        send_register_response(p_u2f_context);
        fido_log_debug("ble_u2f_register end ");
    }
}
