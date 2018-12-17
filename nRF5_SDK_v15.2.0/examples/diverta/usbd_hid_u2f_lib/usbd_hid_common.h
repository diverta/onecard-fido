/* 
 * File:   usbd_hid_common.h
 * Author: makmorit
 *
 * Created on 2018/12/17, 13:23
 */

#ifndef USBD_HID_COMMON_H
#define USBD_HID_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

// USB HIDサービスのメッセージ長に関する定義
#define USBD_HID_PACKET_SIZE 64
#define USBD_HID_INIT_PAYLOAD_SIZE (USBD_HID_PACKET_SIZE-7)
#define USBD_HID_CONT_PAYLOAD_SIZE (USBD_HID_PACKET_SIZE-5)

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

//
// 関数群
//
void     init_CID(void);
uint32_t get_incremented_CID(void);
uint32_t get_CID(uint8_t *cid);
void     set_CID(uint8_t *cid, uint32_t _CID);
size_t   get_payload_length(USB_HID_MSG_T *recv_msg);

void     dump_hid_init_packet(char *msg_header, size_t size, USB_HID_MSG_T *recv_msg);
void     dump_hid_cont_packet(char *msg_header, size_t size, USB_HID_MSG_T *recv_msg);

#ifdef __cplusplus
}
#endif

#endif /* USBD_HID_COMMON_H */

