#include "sdk_common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// for logging informations
#define NRF_LOG_MODULE_NAME ble_u2f_util
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for send ble response
#include "ble_u2f.h"
#include "ble_u2f_status.h"

// レスポンス編集エリア
static uint8_t  data_buffer[2];
static uint32_t data_buffer_length;

// 署名ベースおよび署名を編集するための作業領域（固定長）
#define SIGNATURE_BASE_BUFFER_LENGTH 256
static uint8_t signature_data_buffer[SIGNATURE_BASE_BUFFER_LENGTH];

// U2F Register/Authenticationメッセージを編集するための作業領域（固定長）
#define RESPONSE_DATA_BUFFER_LENGTH 2048
static uint8_t response_message_buffer[RESPONSE_DATA_BUFFER_LENGTH];

void ble_u2f_send_success_response(ble_u2f_context_t *p_u2f_context)
{
    // レスポンスを生成
    uint8_t command_for_response = p_u2f_context->p_ble_header->CMD;
    fido_set_status_word(data_buffer, U2F_SW_NO_ERROR);
    data_buffer_length = 2;

    // 生成したレスポンスを戻す
    ble_u2f_status_setup(command_for_response, data_buffer, data_buffer_length);
    ble_u2f_status_response_send(p_u2f_context->p_u2f);
}

void ble_u2f_send_error_response(ble_u2f_context_t *p_u2f_context, uint16_t err_status_word)
{    
    // コマンドを格納
    uint8_t command_for_response = p_u2f_context->p_ble_header->CMD;

    // ステータスワードを格納
    fido_set_status_word(data_buffer, err_status_word);
    data_buffer_length = 2;
    
    // レスポンスを送信
    ble_u2f_status_setup(command_for_response, data_buffer, data_buffer_length);
    ble_u2f_status_response_send(p_u2f_context->p_u2f);
}

void ble_u2f_send_command_error_response(ble_u2f_context_t *p_u2f_context, uint8_t err_code)
{
    // コマンドを格納
    uint8_t command_for_response = U2F_COMMAND_ERROR;

    // エラーコードを格納
    data_buffer[0]     = err_code;
    data_buffer_length = 1;

    // レスポンスを送信
    ble_u2f_status_setup(command_for_response, data_buffer, data_buffer_length);
    ble_u2f_status_response_send(p_u2f_context->p_u2f);
}

void ble_u2f_send_keepalive_response(ble_u2f_context_t *p_u2f_context)
{
    // コマンドを格納
    uint8_t command_for_response = U2F_COMMAND_KEEPALIVE;

    // エラーコードを格納
    data_buffer[0]     = p_u2f_context->keepalive_status_byte;
    data_buffer_length = 1;

    // レスポンスを送信
    ble_u2f_status_setup(command_for_response, data_buffer, data_buffer_length);
    ble_u2f_status_response_send(p_u2f_context->p_u2f);
}

bool ble_u2f_signature_data_allocate(ble_u2f_context_t *p_u2f_context)
{
    // 署名ベース／署名作業領域の参照先と最大バイト数を保持
    p_u2f_context->signature_data_buffer        = signature_data_buffer;
    p_u2f_context->signature_data_buffer_length = SIGNATURE_BASE_BUFFER_LENGTH;
    NRF_LOG_DEBUG("signature_data_buffer allocated (%d bytes) ", SIGNATURE_BASE_BUFFER_LENGTH);

    // 確保領域は0で初期化
    memset(signature_data_buffer, 0, SIGNATURE_BASE_BUFFER_LENGTH);
    return true;
}


bool ble_u2f_response_message_allocate(ble_u2f_context_t *p_u2f_context)
{
    // U2F Register/Authenticationメッセージ作業領域の参照先と最大バイト数を保持
    p_u2f_context->response_message_buffer        = response_message_buffer;
    p_u2f_context->response_message_buffer_length = RESPONSE_DATA_BUFFER_LENGTH;
    NRF_LOG_DEBUG("response_message_buffer allocated (%d bytes) ", RESPONSE_DATA_BUFFER_LENGTH);

    // 確保領域は0で初期化
    memset(response_message_buffer, 0, RESPONSE_DATA_BUFFER_LENGTH);
    return true;
}
