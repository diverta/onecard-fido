/* 
 * File:   usbd_cdc_logger_interval_timer.h
 * Author: makmorit
 *
 * Created on 2019/03/05, 10:50
 */
#ifndef USBD_CDC_LOGGER_INTERVAL_TIMER_H__
#define USBD_CDC_LOGGER_INTERVAL_TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif

void     usbd_cdc_logger_interval_timer_start(void);
void     usbd_cdc_logger_interval_timer_stop(void);
uint32_t usbd_cdc_logger_serial_second(void);

#ifdef __cplusplus
}
#endif

#endif // USBD_CDC_LOGGER_INTERVAL_TIMER_H__

/** @} */
