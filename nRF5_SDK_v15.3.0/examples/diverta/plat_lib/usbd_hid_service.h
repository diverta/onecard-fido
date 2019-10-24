/* 
 * File:   usbd_hid_service.h
 * Author: makmorit
 *
 * Created on 2018/11/06, 14:21
 */
#ifndef USBD_HID_SERVICE_H
#define USBD_HID_SERVICE_H

#include "app_usbd_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void usbd_init(void);
void usbd_cdc_init(void);
void usbd_hid_init(void (*event_handler_)(app_usbd_event_type_t event));
void usbd_hid_do_process(void);

#ifdef __cplusplus
}
#endif

#endif /* USBD_HID_SERVICE_H */
