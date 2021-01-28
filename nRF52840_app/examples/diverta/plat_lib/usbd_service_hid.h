/* 
 * File:   usbd_service_hid.h
 * Author: makmorit
 *
 * Created on 2020/06/09, 14:47
 */
#ifndef USBD_SERVICE_HID_H
#define USBD_SERVICE_HID_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void usbd_hid_init(void);
void usbd_hid_frame_send(uint8_t *buffer_for_send, size_t size);
void usbd_service_hid_do_process(bool process);

#ifdef __cplusplus
}
#endif

#endif /* USBD_SERVICE_HID_H */
