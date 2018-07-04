#include <stdio.h>

#include "mbed.h"
#include "U2FHID.h"

//
// 受信関連処理
//
bool receive_xfer_response_data(void)
{
    static size_t pos;
    static size_t payload_len;
    u2f_request_length = 0;

    if (recv_report.length == 0) {
        return false;
    }

    size_t init_payload_size = 25;
    size_t cont_payload_size = 27;
    U2F_HID_MSG *req = (U2F_HID_MSG *)recv_report.data;

    if (U2FHID_IS_INIT(req->pkt.init.cmd)) {
        // payload長を取得
        payload_len = get_payload_length(req);

        // リクエストデータ領域に格納
        pos = (payload_len < init_payload_size) ? payload_len : init_payload_size;
        memset(&u2f_request_buffer, 0, sizeof(HID_REPORT));
        memcpy(u2f_request_buffer, req->pkt.init.payload, pos);

        dump_hid_init_packet("Recv ", recv_report.length, req, pos);
        
   } else {

        // リクエストデータ領域に格納
        size_t remain = payload_len - pos;
        size_t cnt = (remain < cont_payload_size) ? remain : cont_payload_size;
        memcpy(u2f_request_buffer + pos, req->pkt.cont.payload, cnt);
        pos += cnt;

        dump_hid_cont_packet("Recv ", recv_report.length, req, cnt);
    }

    // リクエストデータを全て受信したらtrueを戻す
    if (pos == payload_len) {
        u2f_request_length = payload_len;
        return true;
    } else {
        return false;
    }
}

bool receive_request_data(void)
{
    static size_t pos;
    static size_t payload_len;
    u2f_request_length = 0;

    if (recv_report.length == 0) {
        return false;
    }
    
    U2F_HID_MSG *req = (U2F_HID_MSG *)recv_report.data;

    if (U2FHID_IS_INIT(req->pkt.init.cmd)) {
        // payload長を取得
        payload_len = get_payload_length(req);
        dump_hid_init_packet("Recv ", recv_report.length, req, payload_len);

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
