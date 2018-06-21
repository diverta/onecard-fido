#include <stdio.h>

#include "mbed.h"
#include "USBU2FAuthenticator.h"

#define HID_PACKET_SIZE 64

HID_REPORT send_report;
HID_REPORT recv_report;

//
// PC-->mbed: 64バイト
// mbed-->PC: 32バイト
//   割込み処理関連の不具合があり、
//   mbedから64バイト送信するとハングするため
//
USBU2FAuthenticator u2fAuthenticator(64, 32);

//
// 後日、U2F HID関連のファイルに切り出し予定
//
#ifndef U2F_HID_H_
#define U2F_HID_H_

#include <stdint.h>

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

#define U2FHID_INIT_PAYLOAD_SIZE  (HID_PACKET_SIZE-7)
#define U2FHID_CONT_PAYLOAD_SIZE  (HID_PACKET_SIZE-5)
#define U2FHID_MAX_PAYLOAD_SIZE   (7609)

#define U2FHID_IS_INIT(cmd) ((cmd) & 0x80)


typedef struct u2f_hid_msg {
    uint32_t cid;
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

typedef struct u2f_hid_init_response
{
    uint8_t  nonce[8];
    uint32_t cid;
    uint8_t  version_id;
    uint8_t  version_major;
    uint8_t  version_minor;
    uint8_t  version_build;
    uint8_t  cflags;
} U2F_HID_INIT_RES;

#endif // U2F_HID_H_

U2F_HID_INIT_RES  init_res;

void dump_hid_packet(char *msg_header, size_t size, U2F_HID_MSG *recv_msg)
{
    printf("%s(%3d bytes) ", msg_header, size);
    printf("CID: 0x%08x, ", recv_msg->cid);
    printf("CMD: 0x%02x, ", recv_msg->pkt.init.cmd);

    size_t len = ((recv_msg->pkt.init.bcnth << 8) & 0xff00) | (recv_msg->pkt.init.bcntl & 0x00ff);
    printf("Payload(%3d bytes): ", len);
    for(size_t i = 0; i < len; i++) {
        printf("%02x ", recv_msg->pkt.init.payload[i]);
    }
    printf("\r\n");
}

size_t generate_hid_init_response(U2F_HID_MSG *req, U2F_HID_MSG *res)
{
    // 送信パケット格納領域を初期化
    memset(&send_report, 0x00, sizeof(send_report));
    send_report.length = 32;

    // パケットデータを編集 (17 bytes)
    size_t len = 17;
    memcpy(init_res.nonce, req->pkt.init.payload, 8);
    init_res.cid           = 0x01330000;
    init_res.version_id    = 2;
    init_res.version_major = 1;
    init_res.version_minor = 1;
    init_res.version_build = 0;
    init_res.cflags        = 1;
    
    // パケットヘッダーを編集 （7 bytes)
    res->cid            = req->cid;
    res->pkt.init.cmd   = req->pkt.init.cmd;
    res->pkt.init.bcnth = (len >> 8) & 0x00ff;
    res->pkt.init.bcntl = len & 0x00ff;
    memcpy(res->pkt.init.payload, &init_res, len);
    
    // パケットの全長を戻す
    return len + 7;
}

bool send_response_packet()
{
    if (recv_report.length == 0) {
        return true;
    }

    U2F_HID_MSG *res = (U2F_HID_MSG *)send_report.data;
    U2F_HID_MSG *req = (U2F_HID_MSG *)recv_report.data;
    dump_hid_packet("Recv ", recv_report.length, req);
    
    // U2FHID_INITコマンドを処理する
    if (U2FHID_IS_INIT(req->pkt.init.cmd) && req->pkt.init.cmd == U2FHID_INIT) {                
        // パケットを生成して送信
        generate_hid_init_response(req, res);
        if (u2fAuthenticator.send(&send_report) == false) {
            printf("u2fAuthenticator.send failed. \r\n");
            return false;
        }
        dump_hid_packet("Sent ", send_report.length, res);
    }

    return true;
}

int main(void) 
{
    wait(1);
    printf("----- U2F Authenticator sample start -----\r\n");

    while (true) {
        if (u2fAuthenticator.readNB(&recv_report)) {
            send_response_packet();
        }
        wait(0.1);
    }
}