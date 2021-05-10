/* 
 * File:   app_usb_hid.h
 * Author: makmorit
 *
 * Created on 2021/05/04, 11:16
 */
#ifndef APP_USB_HID_H
#define APP_USB_HID_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        app_usb_hid_configured(const uint8_t *param);
bool        app_usb_hid_send_report(uint8_t *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* APP_USB_HID_H */
