/* 
 * File:   usbd_service.h
 * Author: makmorit
 *
 * Created on 2018/11/06, 14:21
 */
#ifndef USBD_SERVICE_H
#define USBD_SERVICE_H

#include "app_usbd_types.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// USB製品関連情報
//
#define USBD_VID                    0xf055
#define USBD_PID                    0x0001
#define USBD_STRINGS_MANUFACTURER   "Diverta Inc."
#define USBD_STRINGS_PRODUCT        "Secure Dongle"
#define USBD_STRINGS_SERIAL         "000000000000"

//
// デバイスディスクリプターの定義
//   app_usbd_core.c 内で、
//   app_usbd_descriptor_device_t 型に加工され、
//   ディスクリプター応答されます。
//
#define APP_USBD_CORE_DEVICE_DESCRIPTOR  {                                                               \
   .bLength = sizeof(app_usbd_descriptor_device_t),    /* descriptor size */                             \
   .bDescriptorType = APP_USBD_DESCRIPTOR_DEVICE,      /* descriptor type */                             \
   .bcdUSB = APP_USBD_BCD_VER_MAKE(2,0),               /* USB BCD version: 2.0 */                        \
   .bDeviceClass = 0,                                  /* device class: 0 - specified by interface */    \
   .bDeviceSubClass = 0,                               /* device subclass: 0 - specified by interface */ \
   .bDeviceProtocol = 0,                               /* device protocol: 0 - specified by interface */ \
   .bMaxPacketSize0 = NRF_DRV_USBD_EPSIZE,             /* endpoint size: fixed to: NRF_DRV_USBD_EPSIZE*/ \
   .idVendor =  USBD_VID,                              /* Vendor ID*/                                    \
   .idProduct = USBD_PID,                              /* Product ID*/                                   \
   .bcdDevice = APP_USBD_BCD_VER_MAKE(0,1),            /* Device version BCD */                          \
   .iManufacturer = APP_USBD_STRING_ID_MANUFACTURER,   /* String ID: manufacturer */                     \
   .iProduct = APP_USBD_STRING_ID_PRODUCT,             /* String ID: product */                          \
   .iSerialNumber = APP_USBD_STRING_ID_SERIAL,         /* String ID: serial */                           \
   .bNumConfigurations = 1                             /* Fixed value: only one configuration supported*/\
}

//
// インターフェースのインデックスを管理
//
// CTAP HID
#define HID_GENERIC_INTERFACE   0
#define HID_GENERIC_EPIN        NRF_DRV_USBD_EPIN1
#define HID_GENERIC_EPOUT       NRF_DRV_USBD_EPOUT1

// CCID
#define CCID_DATA_INTERFACE     1
#define CCID_DATA_EPIN          NRF_DRV_USBD_EPIN2
#define CCID_DATA_EPOUT         NRF_DRV_USBD_EPOUT2

//
// 関数群
//
void usbd_service_init(void (*event_handler_)(app_usbd_event_type_t event));
void usbd_service_start(void);
void usbd_service_do_process(void);

#ifdef __cplusplus
}
#endif

#endif /* USBD_SERVICE_H */
