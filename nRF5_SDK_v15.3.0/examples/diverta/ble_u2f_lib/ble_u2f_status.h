#ifndef BLE_U2F_STATUS_H__
#define BLE_U2F_STATUS_H__

#include "sdk_config.h"

#include "ble.h"
#include "ble_u2f.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// U2F status関連処理
//   CTAP2においても使用します
//
void     ble_u2f_status_setup(uint8_t command_for_response, uint8_t *data_buffer, uint32_t data_buffer_length);
uint32_t ble_u2f_status_response_send(void);
void     ble_u2f_status_on_tx_complete(ble_u2f_t *p_u2f);
void     ble_u2f_status_response_ping(ble_u2f_context_t *p_u2f_context);
void     ble_u2f_status_response_send_retry(void);
//
// メッセージ送信関連処理
//
void     ble_u2f_send_success_response(uint8_t command_for_response);
void     ble_u2f_send_error_response(uint8_t command_for_response, uint16_t err_status_word);
void     ble_u2f_send_command_error_response(uint8_t err_code);
void     ble_u2f_send_keepalive_response(uint8_t keepalive_status_byte);

#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_STATUS_H__

/** @} */
