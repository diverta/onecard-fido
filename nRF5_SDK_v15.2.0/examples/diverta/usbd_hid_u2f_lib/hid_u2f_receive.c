/* 
 * File:   hid_u2f_receive.c
 * Author: makmorit
 *
 * Created on 2018/11/21, 14:21
 */
#include <stdio.h>
#include "hid_u2f_common.h"

// for logging informations
#define NRF_LOG_MODULE_NAME hid_u2f_receive
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

bool hid_u2f_receive_request_data(uint8_t *p_buff, size_t size)
{
    static size_t pos;
    static size_t payload_len;
    u2f_request_length = 0;

    if (size == 0) {
        return false;
    }
    
    U2F_HID_MSG *req = (U2F_HID_MSG *)p_buff;

    if (U2FHID_IS_INIT(req->pkt.init.cmd)) {
        // payload長を取得
        payload_len = get_payload_length(req);
        dump_hid_init_packet("Recv ", size, req, payload_len);

        // リクエストデータ領域に格納
        pos = (payload_len < U2FHID_INIT_PAYLOAD_SIZE) ? payload_len : U2FHID_INIT_PAYLOAD_SIZE;
        memset(&u2f_request_buffer, 0, sizeof(u2f_request_buffer));
        memcpy(u2f_request_buffer, req->pkt.init.payload, pos);

        // CID、CMDを保持
        uint32_t cid = get_CID(req->cid);
        if (cid != U2FHID_RESERVED_CID) {
            CID_for_session = cid;
        }
        CMD_for_session = req->pkt.init.cmd;
        
        // U2Fクライアントからの最初のリクエスト受領時は
        // ステータスを設定（リクエスト受領開始）
        if ((cid != U2FHID_BROADCAST) && (cid != U2FHID_RESERVED_CID)) {
            //u2f_process_state_set(U2FPS_RECV_REQ);
        }

    } else {
        dump_hid_cont_packet("Recv ", size, req, payload_len - pos);

        // リクエストデータ領域に格納
        size_t remain = payload_len - pos;
        size_t cnt = (remain < U2FHID_CONT_PAYLOAD_SIZE) ? remain : U2FHID_CONT_PAYLOAD_SIZE;
        memcpy(u2f_request_buffer + pos, req->pkt.cont.payload, cnt);
        pos += cnt;
    }

    // リクエストデータを全て受信したらtrueを戻す
    if (pos == payload_len) {
        u2f_request_length = payload_len;
        
        // U2F　Helperからのレスポンス受領時の処理
        // （タイマーキャンセル、LED消灯）
        uint32_t cid = get_CID(req->cid);
        if (cid == U2FHID_RESERVED_CID) {
            //u2f_process_state_on_receive_response();
        }
        
        return true;
    } else {
        return false;
    }
}
