/* 
 * File:   hid_u2f_send.h
 * Author: makmorit
 *
 * Created on 2018/11/21, 14:21
 */

#ifndef HID_U2F_SEND_H
#define HID_U2F_SEND_H

#ifdef __cplusplus
extern "C" {
#endif

void send_hid_input_report(uint32_t cid, uint8_t cmd, uint8_t *payload_data, size_t payload_length);

#ifdef __cplusplus
}
#endif

#endif /* HID_U2F_SEND_H */

