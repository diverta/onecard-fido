/* 
 * File:   hid_ctap2_command.h
 * Author: makmorit
 *
 * Created on 2018/12/18, 13:36
 */
#ifndef HID_CTAP2_COMMAND_H
#define HID_CTAP2_COMMAND_H

#include "fds.h"

#ifdef __cplusplus
extern "C" {
#endif

void hid_ctap2_command_init(void);
void hid_ctap2_command_send_response(uint8_t ctap2_status, size_t length);
void hid_ctap2_command_cbor(void);
void hid_ctap2_command_cbor_send_response(fds_evt_t const *const p_evt);
void hid_ctap2_command_cbor_report_sent(void);
void hid_ctap2_command_tup_cancel(void);
void hid_ctap2_command_cancel(void);
void hid_ctap2_command_keepalive_timer_handler(void);
bool hid_ctap2_command_on_mainsw_event(void);
bool hid_ctap2_command_on_mainsw_long_push_event(void);
void hid_ctap2_command_on_ble_nus_connected(void);

#ifdef __cplusplus
}
#endif

#endif /* HID_CTAP2_COMMAND_H */

