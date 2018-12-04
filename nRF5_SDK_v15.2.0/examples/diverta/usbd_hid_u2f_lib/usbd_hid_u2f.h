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
void usbd_hid_init(void);
void usbd_hid_u2f_do_process(void);
void usbd_hid_u2f_frame_send(uint8_t *buffer_for_send, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* USBD_HID_U2F_H */

