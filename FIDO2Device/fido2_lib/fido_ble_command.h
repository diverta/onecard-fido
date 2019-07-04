/* 
 * File:   fido_ble_command.h
 * Author: makmorit
 *
 * Created on 2019/06/26, 16:12
 */
#ifndef FIDO_BLE_COMMAND_H
#define FIDO_BLE_COMMAND_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void fido_ble_command_send_status_response(uint8_t cmd, uint8_t status_code);
void fido_ble_command_on_request_received(void);
void fido_ble_command_on_response_send_completed(void);
void fido_ble_command_on_request_started(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BLE_COMMAND_H */
