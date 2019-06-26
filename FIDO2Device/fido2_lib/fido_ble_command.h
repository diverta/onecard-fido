/* 
 * File:   fido_ble_command.h
 * Author: makmorit
 *
 * Created on 2019/06/26, 16:12
 */
#ifndef FIDO_BLE_COMMAND_H
#define FIDO_BLE_COMMAND_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool fido_ble_command_on_mainsw_event(void);
void fido_ble_command_on_request_received(void);
void fido_ble_command_set_change_pairing_mode(void);
void fido_ble_command_on_fs_evt(void const *p_evt);
void fido_ble_command_keepalive_timer_handler(void *p_context);
void fido_ble_command_on_response_send_completed(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BLE_COMMAND_H */
