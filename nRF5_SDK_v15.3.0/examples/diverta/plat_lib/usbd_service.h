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

// CDC（現在機能閉塞中）
#define CDC_ACM_COMM_INTERFACE  2
#define CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN3

#define CDC_ACM_DATA_INTERFACE  3
#define CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN4
#define CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT4

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
