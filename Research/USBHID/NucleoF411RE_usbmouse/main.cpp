#include <stdio.h>

#include "mbed.h"
#include "USBU2FAuthenticator.h"
#include "U2FHID.h"

//
// PC-->mbed: 64バイト
// mbed-->PC: 32バイト
//   割込み処理関連の不具合があり、
//   mbedから64バイト送信するとハングするため
//
USBU2FAuthenticator u2fAuthenticator(64, 32);

//
// HID送信／受信パケット格納領域
//
HID_REPORT send_report;
HID_REPORT recv_report;

//
// チャネルID関連
//
uint8_t    CMD;
uint32_t   CID;


size_t get_payload_length(U2F_HID_MSG *recv_msg)
{
    return ((recv_msg->pkt.init.bcnth << 8) & 0xff00) | (recv_msg->pkt.init.bcntl & 0x00ff);
}

void dump_hid_init_packet(char *msg_header, size_t size, U2F_HID_MSG *recv_msg)
{
    printf("%s(%3d bytes) ", msg_header, size);
    printf("CID: 0x%08x, ",  get_CID(recv_msg->cid));
    printf("CMD: 0x%02x, ",  recv_msg->pkt.init.cmd);

    size_t len = get_payload_length(recv_msg);
    printf("Payload(%3d bytes): ", len);
    
    size_t cnt = (len < U2FHID_INIT_PAYLOAD_SIZE) ? len : U2FHID_INIT_PAYLOAD_SIZE;
    for(size_t i = 0; i < cnt; i++) {
        printf("%02x ", recv_msg->pkt.init.payload[i]);
    }
    printf("\r\n");
}

void dump_hid_cont_packet(char *msg_header, size_t size, U2F_HID_MSG *recv_msg, size_t remain)
{
    printf("%s(%3d bytes) ", msg_header, size);
    printf("CID: 0x%08x, ",  get_CID(recv_msg->cid));
    printf("SEQ: 0x%02d, ",  recv_msg->pkt.cont.seq);
    
    size_t cnt = (remain < U2FHID_CONT_PAYLOAD_SIZE) ? remain : U2FHID_CONT_PAYLOAD_SIZE;
    for(size_t i = 0; i < cnt; i++) {
        printf("%02x ", recv_msg->pkt.cont.payload[i]);
    }
    printf("\r\n");
}

void set_send_report_data(uint8_t *payload_data, size_t payload_length)
{
    // 送信パケット格納領域を初期化
    memset(&send_report, 0x00, sizeof(send_report));
    send_report.length = 32;

    // パケット格納領域を取得
    U2F_HID_MSG *res = (U2F_HID_MSG *)send_report.data;

    // パケットヘッダーを編集 （7 bytes)
    set_CID(res->cid, CID);
    res->pkt.init.cmd   = CMD;
    res->pkt.init.bcnth = (payload_length >> 8) & 0x00ff;
    res->pkt.init.bcntl = payload_length & 0x00ff;
    
    // パケットデータを設定
    memcpy(res->pkt.init.payload, payload_data, payload_length);
}

bool send_hid_report()
{
    U2F_HID_MSG *res = (U2F_HID_MSG *)send_report.data;
    dump_hid_init_packet("Sent ", send_report.length, res);

    if (u2fAuthenticator.send(&send_report) == false) {
        printf("u2fAuthenticator.send failed. \r\n");
        return false;
    }
    return true;
}

bool send_xfer_report()
{
    size_t xfer_data_max = 25;
    size_t xfer_data_len;
    size_t remaining;
    uint8_t seq = 0;
    
    for (size_t i = 0; i < u2f_request_length; i += xfer_data_max) {
        // 送信パケット格納領域を初期化
        memset(&send_report, 0x00, sizeof(send_report));
        send_report.length = 32;

        // パケット格納領域を取得
        U2F_HID_MSG *res = (U2F_HID_MSG *)send_report.data;

        // データ長
        remaining = u2f_request_length - i;
        xfer_data_len = (remaining < xfer_data_max) ? remaining : xfer_data_max;

        // パケットヘッダーを編集 （7 bytes)
        // U2F管理ツールのチャネルIDは 0x00 とする
        set_CID(res->cid, 0x00);
        res->pkt.init.cmd   = seq++;
        res->pkt.init.bcnth = (xfer_data_len >> 8) & 0x00ff;
        res->pkt.init.bcntl = xfer_data_len & 0x00ff;
    
        // パケットデータを設定
        memcpy(res->pkt.init.payload, u2f_request_buffer + i, xfer_data_len);

        // パケットをU2F管理ツールへ転送
        if (u2fAuthenticator.send(&send_report) == false) {
            printf("u2fAuthenticator.send failed. \r\n");
            return false;
        }
        dump_hid_init_packet("Xfer ", send_report.length, res);
    }

    return true;
}

bool send_response_packet()
{
    if (CMD == U2FHID_INIT) {
        // レスポンスデータを送信パケットに設定
        generate_hid_init_response();
        set_send_report_data(u2f_response_buffer, u2f_response_length);
        if (send_hid_report() == false) {
            return false;
        }
    }

    if (CMD == U2FHID_MSG) {
        // u2f_request_buffer の先頭バイトを参照
        //   [0]CLA [1]INS [2]P1 3[P2]
        uint8_t ins = u2f_request_buffer[1];
        if (ins == U2F_VERSION) {
            // レスポンスデータを送信パケットに設定
            generate_u2f_version_response();
            set_send_report_data(u2f_response_buffer, u2f_response_length);
            if (send_hid_report() == false) {
                return false;
            }
        }
        if (ins == U2F_REGISTER) {
            // TODO: 
            // リクエストデータをU2F管理ツールに転送
            if (send_xfer_report() == false) {
                return false;
            }
        }
    }

    return true;
}

bool receive_request_data()
{
    static size_t pos;
    static size_t payload_len;
    u2f_request_length = 0;

    if (recv_report.length == 0) {
        return false;
    }
    
    // チャネルIDを判定する
    U2F_HID_MSG *req = (U2F_HID_MSG *)recv_report.data;
    if (get_CID(req->cid) == U2FHID_RESERVED_CID) {
        // TODO: 
        //   U2F管理ツールからのリクエストの場合スキップ
        //   （後日実装予定）
        printf("receive_request_data: U2F Maintenance Tool is not suppored\r\n");
        return false;
    }

    if (U2FHID_IS_INIT(req->pkt.init.cmd)) {
        dump_hid_init_packet("Recv ", recv_report.length, req);

        // payload長を取得
        payload_len = get_payload_length(req);

        // リクエストデータ領域に格納
        pos = (payload_len < U2FHID_INIT_PAYLOAD_SIZE) ? payload_len : U2FHID_INIT_PAYLOAD_SIZE;
        memset(&u2f_request_buffer, 0, sizeof(HID_REPORT));
        memcpy(u2f_request_buffer, req->pkt.init.payload, pos);

        // CID、CMDを保持
        CID = get_CID(req->cid);
        CMD = req->pkt.init.cmd;

    } else {
        dump_hid_cont_packet("Recv ", recv_report.length, req, payload_len - pos);

        // リクエストデータ領域に格納
        size_t remain = payload_len - pos;
        size_t cnt = (remain < U2FHID_CONT_PAYLOAD_SIZE) ? remain : U2FHID_CONT_PAYLOAD_SIZE;
        memcpy(u2f_request_buffer + pos, req->pkt.cont.payload, cnt);
        pos += cnt;
    }

    // リクエストデータを全て受信したらtrueを戻す
    if (pos == payload_len) {
        u2f_request_length = payload_len;
        return true;
    } else {
        return false;
    }
}

int main(void) 
{
    wait(1);
    printf("----- U2F Authenticator sample start -----\r\n");

    while (true) {
        if (u2fAuthenticator.readNB(&recv_report)) {
            if (receive_request_data() == true) {
                // リクエストを全て受領したらレスポンス
                send_response_packet();
            }
        }
        wait(0.1);
    }
}