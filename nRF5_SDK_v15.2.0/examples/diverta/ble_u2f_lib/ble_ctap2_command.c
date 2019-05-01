/* 
 * File:   ble_ctap2_command.c
 * Author: makmorit
 *
 * Created on 2019/04/30, 13:48
 */
#include "sdk_common.h"

#include "ctap2_common.h"
#include "ctap2_cbor_authgetinfo.h"
#include "fido_common.h"

// for BLE transport
#include "ble_u2f_command.h"
#include "ble_u2f_status.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ble_ctap2_command
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

//
// CTAP2レスポンスデータ格納領域
// （コマンド共通）
//
static uint8_t response_buffer[CTAP2_MAX_MESSAGE_SIZE];
static size_t  response_length;

void ble_ctap2_command_send_response(uint8_t ctap2_status, size_t length)
{
    // コマンドを格納
    ble_u2f_context_t *p_u2f_context = get_ble_u2f_context();
    uint8_t command_for_response = p_u2f_context->p_ble_header->CMD;
    // １バイトめにステータスコードをセット
    response_buffer[0] = ctap2_status;
    response_length = length;
    
    // レスポンスを送信
    ble_u2f_status_setup(command_for_response, response_buffer, response_length);
    ble_u2f_status_response_send(p_u2f_context->p_u2f);
}

static void send_ctap2_command_error_response(uint8_t ctap2_status) 
{
    // CTAP2 CBORコマンドに対応する
    // レスポンスデータを送信パケットに設定し送信
    //   エラーなので送信バイト数＝１
    ble_ctap2_command_send_response(ctap2_status, 1);
}

void ble_ctap2_authenticator_get_info(void)
{
    // レスポンスの先頭１バイトはステータスコードであるため、
    // ２バイトめからCBORレスポンスをセットさせるようにする
    uint8_t  ctap2_status;
    uint8_t *cbor_data_buffer = response_buffer + 1;
    size_t   cbor_data_length = sizeof(response_buffer) - 1;
    
    // authenticatorGetInfoレスポンスをエンコード
    ctap2_status = ctap2_cbor_authgetinfo_encode_request(cbor_data_buffer, &cbor_data_length);
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        send_ctap2_command_error_response(ctap2_status);
        return;
    }

    // レスポンスデータを転送
    ble_ctap2_command_send_response(ctap2_status, cbor_data_length + 1);
}
