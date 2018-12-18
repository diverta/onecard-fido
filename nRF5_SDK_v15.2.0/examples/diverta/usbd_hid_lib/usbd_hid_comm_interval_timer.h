/* 
 * File:   usbd_hid_comm_interval_timer.h
 * Author: makmorit
 *
 * Created on 2018/11/26, 10:50
 */
#ifndef USBD_HID_COMM_INTERVAL_TIMER_H__
#define USBD_HID_COMM_INTERVAL_TIMER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


void usbd_hid_comm_interval_timer_start(void);
void usbd_hid_comm_interval_timer_stop(void);


#ifdef __cplusplus
}
#endif

#endif // USBD_HID_COMM_INTERVAL_TIMER_H__

/** @} */
