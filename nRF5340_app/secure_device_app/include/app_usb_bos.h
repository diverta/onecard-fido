/* 
 * File:   app_usb_bos.h
 * Author: makmorit
 *
 * Created on 2022/07/05, 16:49
 */
#ifndef APP_USB_BOS_H
#define APP_USB_BOS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
uint8_t    *app_usb_bos_msos2_descriptor(void);
size_t      app_usb_bos_msos2_descriptor_size(void);
void        app_usb_bos_register_caps(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_USB_BOS_H */
