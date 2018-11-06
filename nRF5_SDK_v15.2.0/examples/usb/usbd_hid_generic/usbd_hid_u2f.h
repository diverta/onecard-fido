/* 
 * File:   usbd_hid_u2f.h
 * Author: makmorit
 *
 * Created on 2018/11/06, 14:21
 */

#ifndef USBD_HID_U2F_H
#define USBD_HID_U2F_H

#ifdef __cplusplus
extern "C" {
#endif

void usbd_init(void);
void usbd_input_report_send(void);

#ifdef __cplusplus
}
#endif

#endif /* USBD_HID_U2F_H */

