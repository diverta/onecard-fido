/* 
 * File:   fido_ble_send.h
 * Author: makmorit
 *
 * Created on 2019/06/27, 9:49
 */
#ifndef FIDO_BLE_SEND_H
#define FIDO_BLE_SEND_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// U2F status関連処理
//   CTAP2においても使用します
//
void fido_ble_send_on_tx_complete(void);
void fido_ble_send_response_retry(void);

//
// メッセージ送信関連処理
//
void fido_ble_send_response_data(uint8_t command_for_response, uint8_t *data_buffer, uint32_t data_buffer_length);
void fido_ble_send_success_response(uint8_t command_for_response);
void fido_ble_send_error_response(uint8_t command_for_response, uint16_t err_status_word);
void fido_ble_send_command_error_response(uint8_t err_code);
void fido_ble_send_keepalive_response(uint8_t keepalive_status_byte);
void fido_ble_send_ping_response(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BLE_SEND_H */
