#include "sdk_common.h"

#include <stdio.h>
#include <string.h>

// for logging informations
#define NRF_LOG_MODULE_NAME ble_u2f_version
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#include "fido_common.h"
#include "ble_u2f.h"
#include "ble_u2f_util.h"
#include "ble_u2f_status.h"

// レスポンスデータ編集用領域
static uint8_t u2f_version_data_buffer[8];

// FIDOアライアンスが制定したバージョン文字列を保持
static uint8_t u2f_version[] = U2F_V2_VERSION_STRING;
static uint8_t u2f_version_length = 6;

void ble_u2f_version_do_process(ble_u2f_context_t *p_u2f_context)
{
    NRF_LOG_DEBUG("ble_u2f_version start ");

    // コマンド、ステータスワードを設定
    uint8_t command_for_response = p_u2f_context->p_ble_header->CMD;
    uint16_t status_word = U2F_SW_NO_ERROR;

    // レスポンスデータを格納
    memcpy(u2f_version_data_buffer, u2f_version, u2f_version_length);

    if (p_u2f_context->p_apdu->Le < 6) {
        // Leを確認し、6バイトでなかったら
        // エラーレスポンスを送信し終了
        NRF_LOG_ERROR("Response message length(6) exceeds Le(%d) ", p_u2f_context->p_apdu->Le);
        ble_u2f_send_error_response(p_u2f_context, U2F_SW_WRONG_LENGTH);
        return;
    }
    
    // ステータスワードを格納
    fido_set_status_word(u2f_version_data_buffer + u2f_version_length, status_word);

    // レスポンスを送信
    ble_u2f_status_setup(command_for_response, u2f_version_data_buffer, sizeof(u2f_version_data_buffer));
    ble_u2f_status_response_send(p_u2f_context->p_u2f);
    NRF_LOG_DEBUG("ble_u2f_version end ");
}
