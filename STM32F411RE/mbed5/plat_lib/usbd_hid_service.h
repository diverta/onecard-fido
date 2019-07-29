/* 
 * File:   usbd_hid_service.h
 * Author: makmorit
 *
 * Created on 2019/07/29, 14:30
 */
#ifndef USBD_HID_SERVICE_H
#define USBD_HID_SERVICE_H

void usbd_hid_init(void);
void usbd_hid_do_process(void);

//
// C --> CPP 呼出用インターフェース
//
void _usbd_hid_frame_send(uint8_t *buffer_for_send, size_t size);

#endif /* USBD_HID_SERVICE_H */
