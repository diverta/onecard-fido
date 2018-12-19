/* 
 * File:   hid_fido_send.h
 * Author: makmorit
 *
 * Created on 2018/11/21, 14:21
 */

#ifndef HID_FIDO_SEND_H
#define HID_FIDO_SEND_H

#ifdef __cplusplus
extern "C" {
#endif

void hid_fido_send_input_report_complete();
void hid_fido_send_command_response(uint32_t cid, uint8_t cmd, uint8_t *response_buffer, size_t response_length);
void hid_fido_send_error_command_response(uint32_t cid, uint8_t error_cmd, uint8_t error_code);

#ifdef __cplusplus
}
#endif

#endif /* HID_FIDO_SEND_H */

