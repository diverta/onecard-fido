/* 
 * File:   fido_hid_send.h
 * Author: makmorit
 *
 * Created on 2018/11/21, 14:21
 */
#ifndef FIDO_HID_SEND_H
#define FIDO_HID_SEND_H

#ifdef __cplusplus
extern "C" {
#endif

void fido_hid_send_input_report_complete();
void fido_hid_send_command_response(uint32_t cid, uint8_t cmd, uint8_t *response_buffer, size_t response_length);
void fido_hid_send_command_response_no_payload(uint32_t cid, uint8_t cmd);
void fido_hid_send_command_response_no_callback(uint32_t cid, uint8_t cmd, uint8_t status_code);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_HID_SEND_H */
