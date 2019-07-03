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
void fido_ble_send_command_response(uint8_t command_for_response, uint8_t *data_buffer, uint32_t data_buffer_length);
void fido_ble_send_command_response_no_callback(uint8_t cmd, uint8_t status_code);
void fido_ble_send_status_word(uint8_t command_for_response, uint16_t err_status_word);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BLE_SEND_H */
