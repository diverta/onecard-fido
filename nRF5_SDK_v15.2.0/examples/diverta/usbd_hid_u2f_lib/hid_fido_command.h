/* 
 * File:   hid_fido_command.h
 * Author: makmorit
 *
 * Created on 2018/12/17, 15:11
 */

#ifndef HID_FIDO_COMMAND_H
#define HID_FIDO_COMMAND_H

#include <stdint.h>
#include <stdbool.h>

#include "fds.h"
#include "peer_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

void hid_u2f_command_on_report_received(uint8_t *request_frame_buffer, size_t request_frame_number);
void hid_u2f_command_on_fs_evt(fds_evt_t const *const p_evt);
void hid_u2f_command_on_report_sent(void);

bool hid_u2f_command_on_mainsw_event(void);
bool hid_u2f_command_on_mainsw_long_push_event(void);

void hid_u2f_command_on_process_started(void);
void hid_u2f_command_on_process_ended(void);
void hid_u2f_command_on_process_timedout(void);

bool hid_u2f_command_is_valid(uint8_t command);

#ifdef __cplusplus
}
#endif

#endif /* HID_FIDO_COMMAND_H */

