#include "sdk_common.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ble_u2f_util
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for send ble response
#include "ble_u2f.h"
#include "ble_u2f_command.h"

// 署名ベースおよび署名を編集するための作業領域（固定長）
#define SIGNATURE_BASE_BUFFER_LENGTH 256
static uint8_t signature_data_buffer[SIGNATURE_BASE_BUFFER_LENGTH];

// U2F Register/Authenticationメッセージを編集するための作業領域（固定長）
#define RESPONSE_DATA_BUFFER_LENGTH 2048
static uint8_t response_message_buffer[RESPONSE_DATA_BUFFER_LENGTH];

bool ble_u2f_signature_data_allocate(void)
{
    // 署名ベース／署名作業領域の参照先と最大バイト数を保持
    ble_u2f_context_t *p_u2f_context = get_ble_u2f_context();
    p_u2f_context->signature_data_buffer        = signature_data_buffer;
    p_u2f_context->signature_data_buffer_length = SIGNATURE_BASE_BUFFER_LENGTH;
    NRF_LOG_DEBUG("signature_data_buffer allocated (%d bytes) ", SIGNATURE_BASE_BUFFER_LENGTH);

    // 確保領域は0で初期化
    memset(signature_data_buffer, 0, SIGNATURE_BASE_BUFFER_LENGTH);
    return true;
}

bool ble_u2f_response_message_allocate(void)
{
    // U2F Register/Authenticationメッセージ作業領域の参照先と最大バイト数を保持
    ble_u2f_context_t *p_u2f_context = get_ble_u2f_context();
    p_u2f_context->response_message_buffer        = response_message_buffer;
    p_u2f_context->response_message_buffer_length = RESPONSE_DATA_BUFFER_LENGTH;
    NRF_LOG_DEBUG("response_message_buffer allocated (%d bytes) ", RESPONSE_DATA_BUFFER_LENGTH);

    // 確保領域は0で初期化
    memset(response_message_buffer, 0, RESPONSE_DATA_BUFFER_LENGTH);
    return true;
}
