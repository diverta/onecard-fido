/* 
 * File:   usbd_cdc_logger_process.h
 * Author: makmorit
 *
 * Created on 2019/03/05, 12:53
 */
#ifndef USBD_CDC_LOGGER_PROCESS_H
#define USBD_CDC_LOGGER_PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

void usbd_cdc_logger_process(void);
void usbd_cdc_logger_process_set_rssi(int8_t rssi_val);

#ifdef __cplusplus
}
#endif

#endif /* USBD_CDC_LOGGER_PROCESS_H */
