/* 
 * File:   usbd_service.h
 * Author: makmorit
 *
 * Created on 2018/11/06, 14:21
 */
#ifndef USBD_SERVICE_H
#define USBD_SERVICE_H

#include "app_usbd_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void usbd_cdc_init(void);
bool usbd_cdc_port_is_open(void);
void usbd_hid_init(void);
void usbd_service_init(void (*event_handler_)(app_usbd_event_type_t event));
void usbd_service_do_process(void);

#ifdef __cplusplus
}
#endif

#endif /* USBD_SERVICE_H */
