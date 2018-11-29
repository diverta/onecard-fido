/* 
 * File:   hid_u2f_receive.h
 * Author: makmorit
 *
 * Created on 2018/11/21, 14:21
 */

#ifndef HID_U2F_RECEIVE_H
#define HID_U2F_RECEIVE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "u2f.h"

HID_HEADER_T *hid_u2f_receive_hid_header(void);
U2F_APDU_T   *hid_u2f_receive_apdu(void);

bool hid_u2f_receive_request_data(uint8_t *p_buff, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* HID_U2F_RECEIVE_H */

