/* 
 * File:   hid_u2f_send.c
 * Author: makmorit
 *
 * Created on 2018/11/21, 14:21
 */
#include <stdio.h>

#include "usbd_hid_u2f.h"
#include "hid_u2f_common.h"

// for logging informations
#define NRF_LOG_MODULE_NAME hid_u2f_send
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// U2F HIDリクエストデータ格納領域
static uint8_t hid_u2f_send_buffer[U2FHID_PACKET_SIZE];
static size_t  hid_u2f_send_buffer_length;

static void generate_hid_input_report(uint8_t *payload_data, size_t payload_length, 
                               size_t offset, size_t xfer_data_len, 
                               uint32_t cid, uint8_t cmd)
{
    static uint8_t seq;

    // 送信パケット格納領域を初期化
    memset(&hid_u2f_send_buffer, 0x00, sizeof(hid_u2f_send_buffer));
    hid_u2f_send_buffer_length = U2FHID_PACKET_SIZE;

    // パケット格納領域を取得
    U2F_HID_MSG *res = (U2F_HID_MSG *)hid_u2f_send_buffer;

    if (offset == 0) {
        // チャネルID、CMD、データ長を設定
        set_CID(res->cid, cid);
        res->pkt.init.cmd   = cmd;
        res->pkt.init.bcnth = (payload_length >> 8) & 0x00ff;
        res->pkt.init.bcntl = payload_length & 0x00ff;

        // パケットデータを設定
        memcpy(res->pkt.init.payload, payload_data + offset, xfer_data_len);
        dump_hid_init_packet("Send ", hid_u2f_send_buffer_length, res, xfer_data_len);

        // シーケンスを初期化
        seq = 0;

    } else {
        // チャネルID、シーケンスを設定
        set_CID(res->cid, cid);
        res->pkt.cont.seq = seq++;

        // パケットデータを設定
        memcpy(res->pkt.cont.payload, payload_data + offset, xfer_data_len);
        dump_hid_cont_packet("Send ", hid_u2f_send_buffer_length, res, xfer_data_len);
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
        xfer_data_max = (i == 0) ? U2FHID_INIT_PAYLOAD_SIZE : U2FHID_CONT_PAYLOAD_SIZE;
        xfer_data_len = (remaining < xfer_data_max) ? remaining : xfer_data_max;

        // パケットを生成
        generate_hid_input_report(payload_data, payload_length, i, xfer_data_len, CID_for_session, CMD_for_session);

        // パケットをU2Fクライアントへ転送
        usbd_hid_u2f_frame_send(hid_u2f_send_buffer, hid_u2f_send_buffer_length);
    }

    return true;
}

bool hid_u2f_send_response_packet(void)
{
    bool ret = true;
    if (CMD_for_session == U2FHID_INIT) {
        // レスポンスデータを送信パケットに設定
        generate_hid_init_response();
        if (send_hid_input_report(u2f_response_buffer, u2f_response_length) == false) {
            return false;
        }
    }

    if (CMD_for_session == U2FHID_MSG) {
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
        if (ins == U2F_REGISTER || ins == U2F_AUTHENTICATE) {
            // リクエストデータをU2F管理ツールに転送
            //if (send_xfer_report(u2f_request_buffer, u2f_request_length) == false) {
            //    return false;
            //}
            // ステータスを設定（リクエスト転送完了）
            //u2f_process_state_set(U2FPS_XFER_REQ);
        }
    }
    
    if (CMD_for_session == U2FHID_ERROR) {
        // レスポンスデータを送信パケットに設定
        generate_u2f_register_response();
        ret = send_hid_input_report(u2f_response_buffer, u2f_response_length);
    }

    // ステータスを設定（レスポンス転送完了）
    //u2f_process_state_set(U2FPS_XFER_RES);
    return ret;
}