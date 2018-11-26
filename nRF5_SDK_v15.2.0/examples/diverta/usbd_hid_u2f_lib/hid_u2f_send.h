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

bool hid_u2f_send_response_packet(void);
void hid_u2f_send_error_response_packet(uint8_t error_code);

#ifdef __cplusplus
}
#endif

#endif /* HID_U2F_SEND_H */

