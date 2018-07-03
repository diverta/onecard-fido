#include <stdio.h>

#include "mbed.h"
#include "U2FHID.h"

//
// 送信関連処理
//
static void generate_hid_input_report(uint8_t *payload_data, size_t payload_length, 
                               size_t offset, size_t xfer_data_len, 
                               uint32_t cid, uint8_t cmd, bool init)
{
    static uint8_t seq;

    // 送信パケット格納領域を初期化
    memset(&send_report, 0x00, sizeof(send_report));
    send_report.length = 32;

    // パケット格納領域を取得
    U2F_HID_MSG *res = (U2F_HID_MSG *)send_report.data;

    if (init) {
        // チャネルID、CMD、データ長を設定
        set_CID(res->cid, cid);
        res->pkt.init.cmd   = cmd;
        res->pkt.init.bcnth = (payload_length >> 8) & 0x00ff;
        res->pkt.init.bcntl = payload_length & 0x00ff;

        // パケットデータを設定
        memcpy(res->pkt.init.payload, payload_data + offset, xfer_data_len);
        dump_hid_init_packet("Send ", send_report.length, res, xfer_data_len);

        // シーケンスを初期化
        seq = 0;

    } else {
        // チャネルID、シーケンスを設定
        set_CID(res->cid, cid);
        res->pkt.cont.seq = seq++;

        // パケットデータを設定
        memcpy(res->pkt.cont.payload, payload_data + offset, xfer_data_len);
        dump_hid_cont_packet("Send ", send_report.length, res, xfer_data_len);
    }
}

static bool send_hid_input_report(uint8_t *payload_data, size_t payload_length)
{
    size_t  xfer_data_max;
    size_t  xfer_data_len;
    size_t  remaining;

    for (size_t i = 0; i < payload_length; i += xfer_data_len) {
        // データ長
        remaining = payload_length - i;
        xfer_data_max = (i == 0) ? 25 : 27;
        xfer_data_len = (remaining < xfer_data_max) ? remaining : xfer_data_max;

        // パケットを生成（i==0ならinitフレーム、それ以外はcontフレームとして生成）
        generate_hid_input_report(payload_data, payload_length, i, xfer_data_len, CID, CMD, (i == 0));

        // パケットをU2Fクライアントへ転送
        if (u2fAuthenticator.send(&send_report) == false) {
            printf("u2fAuthenticator.send failed. \r\n");
            return false;
        }
    }

    return true;
}

static bool send_xfer_report(uint8_t *payload_data, size_t payload_length)
{
    size_t  xfer_data_max = 25;
    size_t  xfer_data_len;
    size_t  remaining;
    uint8_t seq = 0;
    
    for (size_t i = 0; i < payload_length; i += xfer_data_len) {
        // データ長
        remaining = payload_length - i;
        xfer_data_len = (remaining < xfer_data_max) ? remaining : xfer_data_max;

        // パケットを生成（常にinitフレームとして生成）
        generate_hid_input_report(payload_data, xfer_data_len, i, xfer_data_len, 0x00, seq++, true);

        // パケットをU2F管理ツールへ転送
        if (u2fAuthenticator.send(&send_report) == false) {
            printf("u2fAuthenticator.send failed. \r\n");
            return false;
        }
    }

    return true;
}

bool send_response_packet(void)
{
    if (CMD == U2FHID_INIT) {
        // レスポンスデータを送信パケットに設定
        generate_hid_init_response();
        if (send_hid_input_report(u2f_response_buffer, u2f_response_length) == false) {
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
            if (send_hid_input_report(u2f_response_buffer, u2f_response_length) == false) {
                return false;
            }
        }
        if (ins == U2F_REGISTER) {
            // リクエストデータをU2F管理ツールに転送
            if (send_xfer_report(u2f_request_buffer, u2f_request_length) == false) {
                return false;
            }
        }
    }

    if (CMD == U2F_VENDOR_LAST) {
        // レスポンスデータを送信パケットに設定
        CMD = U2FHID_MSG;
        generate_u2f_register_response();
        if (send_hid_input_report(u2f_response_buffer, u2f_response_length) == false) {
            return false;
        }
    }

    return true;
}
