#include <stdio.h>
#include <string.h>

#include "u2f.h"
#include "fido_ble_command.h"
#include "fido_ble_receive.h"
#include "fido_ble_send.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// レスポンスデータ編集用領域
static uint8_t u2f_version_data_buffer[8];

// FIDOアライアンスが制定したバージョン文字列を保持
static uint8_t u2f_version[] = U2F_V2_VERSION_STRING;
static uint8_t u2f_version_length = 6;

void ble_u2f_version_do_process(void)
{
    fido_log_debug("ble_u2f_version start ");

    // コマンド、ステータスワードを設定
    uint8_t command_for_response = fido_ble_receive_header()->CMD;
    uint16_t status_word = U2F_SW_NO_ERROR;

    // レスポンスデータを格納
    memcpy(u2f_version_data_buffer, u2f_version, u2f_version_length);

    if (fido_ble_receive_apdu()->Le < 6) {
        // Leを確認し、6バイトでなかったら
        // エラーレスポンスを送信し終了
        fido_log_error("Response message length(6) exceeds Le(%d) ", fido_ble_receive_apdu()->Le);
        uint8_t cmd = fido_ble_receive_header()->CMD;
        fido_ble_command_send_status_word(cmd, U2F_SW_WRONG_LENGTH);
        return;
    }
    
    // ステータスワードを格納
    fido_set_status_word(u2f_version_data_buffer + u2f_version_length, status_word);

    // レスポンスを送信
    fido_ble_send_command_response(command_for_response, u2f_version_data_buffer, sizeof(u2f_version_data_buffer));
    fido_log_debug("ble_u2f_version end ");
}
