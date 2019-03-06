/* 
 * File:   usbd_cdc_service.h
 * Author: makmorit
 *
 * Created on 2019/03/05, 11:21
 */

#ifndef USBD_CDC_SERVICE_H
#define USBD_CDC_SERVICE_H

#include "sdk_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

void       usbd_init(void);
void       usbd_cdc_init(void);
void       usbd_cdc_do_process(void);
ret_code_t usbd_cdc_buffer_write(const void *p_tx_buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* USBD_CDC_SERVICE_H */

