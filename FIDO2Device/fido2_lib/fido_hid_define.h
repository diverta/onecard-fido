/* 
 * File:   fido_hid_define.h
 * Author: makmorit
 *
 * Created on 2023/01/02, 15:51
 */
#ifndef FIDO_HID_DEFINE_H
#define FIDO_HID_DEFINE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// USB HIDサービスのメッセージ長に関する定義
#define USBD_HID_PACKET_SIZE 64
#define USBD_HID_INIT_PAYLOAD_SIZE (USBD_HID_PACKET_SIZE-7)
#define USBD_HID_CONT_PAYLOAD_SIZE (USBD_HID_PACKET_SIZE-5)
#define USBD_HID_MAX_PAYLOAD_SIZE 2048

// USB HIDサービスで使用するCIDの長さ（4バイト）
#define USBD_HID_CID_LEN 4

// USB HIDサービスで使用するCID
#define USBD_HID_BROADCAST    0xffffffff
#define USBD_HID_INITIAL_CID  0x01003300

// USB HIDメッセージ構造体
//   固定長（64バイト）
typedef struct {
    uint8_t cid[4];
    union {
        struct {
            uint8_t cmd;
            uint8_t bcnth;
            uint8_t bcntl;
            uint8_t payload[USBD_HID_INIT_PAYLOAD_SIZE];
        } init;
        struct {
            uint8_t seq;
            uint8_t payload[USBD_HID_CONT_PAYLOAD_SIZE];
        } cont;
    } pkt;
} USB_HID_MSG_T;

#ifdef __cplusplus
}
#endif

#endif /* FIDO_HID_DEFINE_H */
