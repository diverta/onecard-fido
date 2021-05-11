/* 
 * File:   app_usb_ccid.h
 * Author: makmorit
 *
 * Created on 2021/05/11, 11:57
 */
#ifndef APP_USB_CCID_H
#define APP_USB_CCID_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool        app_usb_ccid_send_data(uint8_t *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* APP_USB_CCID_H */
