/* 
 * File:   hid_u2f_common.h
 * Author: makmorit
 *
 * Created on 2018/11/21, 14:21
 */

#ifndef HID_U2F_COMMON_H
#define HID_U2F_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

// U2F HIDメッセージ長に関する定義
#define U2FHID_PACKET_SIZE 64
#define U2FHID_INIT_PAYLOAD_SIZE (U2FHID_PACKET_SIZE-7)
#define U2FHID_CONT_PAYLOAD_SIZE (U2FHID_PACKET_SIZE-5)

// 先頭フレームかどうかを判定
#define U2FHID_IS_INIT(cmd) ((cmd) & 0x80)

//
// U2F HID関連の定義群
//
#define TYPE_MASK           0x80    // Frame type mask
#define TYPE_INIT           0x80    // Initial frame identifier
#define TYPE_CONT           0x00    // Continuation frame identifier

#define U2FHID_PING         (TYPE_INIT | 0x01)  // Echo data through local processor only
#define U2FHID_MSG          (TYPE_INIT | 0x03)  // Send U2F message frame
#define U2FHID_LOCK         (TYPE_INIT | 0x04)  // Send lock channel command
#define U2FHID_INIT         (TYPE_INIT | 0x06)  // Channel initialization
#define U2FHID_WINK         (TYPE_INIT | 0x08)  // Send device identification wink
#define U2FHID_ERROR        (TYPE_INIT | 0x3f)  // Error response

#define U2FHID_BROADCAST    0xffffffff
#define U2FHID_RESERVED_CID 0x00000000
#define U2FHID_INITIAL_CID  0x01003300

// U2F native commands
#define U2F_REGISTER        0x01
#define U2F_AUTHENTICATE    0x02
#define U2F_VERSION         0x03
#define U2F_VENDOR_FIRST    0xc0
#define U2F_VENDOR_LAST     0xff

// Command status responses
#define U2F_SW_NO_ERROR     0x9000

// U2F HIDメッセージ構造体
//   固定長（64バイト）
typedef struct u2f_hid_msg {
    uint8_t cid[4];
    union {
        struct {
            uint8_t cmd;
            uint8_t bcnth;
            uint8_t bcntl;
            uint8_t payload[U2FHID_INIT_PAYLOAD_SIZE];
        } init;
        struct {
            uint8_t seq;
            uint8_t payload[U2FHID_CONT_PAYLOAD_SIZE];
        } cont;
    } pkt;
} U2F_HID_MSG;

// U2F HIDリクエストデータ格納領域
extern uint8_t u2f_request_buffer[1024];
extern size_t  u2f_request_length;

// U2F HIDレスポンスデータ格納領域
extern uint8_t u2f_response_buffer[1024];
extern size_t  u2f_response_length;

//
// 現在処理中のチャネルID、コマンドを保持
//
extern uint8_t  CMD;
extern uint32_t CID;

//
// 関数群
//
void     init_CID(void);
uint32_t get_CID(uint8_t *cid);
void     set_CID(uint8_t *cid, uint32_t _CID);
size_t   get_payload_length(U2F_HID_MSG *recv_msg);

void     dump_hid_init_packet(char *msg_header, size_t size, U2F_HID_MSG *recv_msg, size_t remain);
void     dump_hid_cont_packet(char *msg_header, size_t size, U2F_HID_MSG *recv_msg, size_t remain);

void     generate_hid_init_response(void);
void     generate_u2f_version_response(void);
void     generate_u2f_register_response(void);

#ifdef __cplusplus
}
#endif

#endif /* HID_U2F_COMMON_H */

