/* 
 * File:   ble_ctap2_command.h
 * Author: makmorit
 *
 * Created on 2019/04/30, 13:48
 */
#ifndef BLE_CTAP2_COMMAND_H
#define BLE_CTAP2_COMMAND_H

#include "fds.h"

#ifdef __cplusplus
extern "C" {
#endif

bool ble_ctap2_command_on_mainsw_event(void);
bool is_ctap2_command_byte(uint8_t command_byte);
void ble_ctap2_command_do_process(void);
void ble_ctap2_command_on_fs_evt(fds_evt_t const *const p_evt);
void ble_ctap2_command_response_sent(void);

#ifdef __cplusplus
}
#endif

#endif /* BLE_CTAP2_COMMAND_H */

