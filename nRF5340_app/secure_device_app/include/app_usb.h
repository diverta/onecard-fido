/* 
 * File:   app_usb.h
 * Author: makmorit
 *
 * Created on 2021/05/04, 11:16
 */
#ifndef APP_USB_H
#define APP_USB_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        app_usb_initialize(void);
bool        app_usb_deinitialize(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_USB_H */
