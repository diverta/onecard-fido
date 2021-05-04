/* 
 * File:   app_usb_hid.h
 * Author: makmorit
 *
 * Created on 2021/05/04, 11:16
 */
#ifndef APP_USB_HID_H
#define APP_USB_HID_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        app_usb_hid_configured(const uint8_t *param);

#ifdef __cplusplus
}
#endif

#endif /* APP_USB_HID_H */
