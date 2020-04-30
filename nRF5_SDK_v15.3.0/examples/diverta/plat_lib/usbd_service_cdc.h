/* 
 * File:   usbd_service_cdc.h
 * Author: makmorit
 *
 * Created on 2020/04/27, 10:27
 */
#ifndef USBD_SERVICE_CDC_H
#define USBD_SERVICE_CDC_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void usbd_cdc_init(void);
bool usbd_cdc_port_is_open(void);
void usbd_service_cdc_do_process(void);

#ifdef __cplusplus
}
#endif

#endif /* USBD_SERVICE_CDC_H */
