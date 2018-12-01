/* 
 * File:   hid_u2f_send.c
 * Author: makmorit
 *
 * Created on 2018/11/21, 14:21
 */
#include <stdio.h>

#include "usbd_hid_u2f.h"
#include "hid_u2f_common.h"
#include "hid_u2f_comm_interval_timer.h"

// for logging informations
#define NRF_LOG_MODULE_NAME hid_u2f_send
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// U2F HIDリクエストデータ格納領域
static uint8_t hid_u2f_send_buffer[U2FHID_PACKET_SIZE];
static size_t  hid_u2f_send_buffer_length;

static struct {
    // 送信済みバイト数、シーケンスを保持
    size_t   sent_length;
    uint8_t  sequence;

    // 送信用ヘッダーに格納するコマンド、データ長、
    // 送信データを保持
    uint32_t cid;
    uint8_t  cmd;
    uint8_t *payload_data;
    size_t   payload_length;

    // 送信がビジー状態かどうかを保持するフラグ
    bool     busy;
} send_info_t;

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
        dump_hid_init_packet("Send ", hid_u2f_send_buffer_length, res);

        // シーケンスを初期化
        seq = 0;

    } else {
        // チャネルID、シーケンスを設定
        set_CID(res->cid, cid);
        res->pkt.cont.seq = seq++;

        // パケットデータを設定
        memcpy(res->pkt.cont.payload, payload_data + offset, xfer_data_len);
        dump_hid_cont_packet("Send ", hid_u2f_send_buffer_length, res);
    }
}

void hid_u2f_send_setup(uint32_t cid, uint8_t cmd, uint8_t *payload_data, size_t payload_length)
{
    // 送信のために必要な情報を保持
    send_info_t.cid = cid;
    send_info_t.cmd = cmd;
    send_info_t.payload_data = payload_data;
    send_info_t.payload_length = payload_length;

    // 送信済みバイト数、シーケンスをゼロクリア
    send_info_t.sent_length = 0;
    send_info_t.sequence = 0;

    // フラグをクリア
    send_info_t.busy = false;
}

void hid_u2f_send_input_report(void)
{
    size_t  xfer_data_max;
    size_t  xfer_data_len;
    size_t  remaining;

    if (send_info_t.payload_length == 0) {
        // レスポンス長が０の場合は何もしない
        return;
    }
    
    for (size_t i = 0; i < send_info_t.payload_length; i += xfer_data_len) {
        // データ長
        remaining = send_info_t.payload_length - i;
        xfer_data_max = (i == 0) ? U2FHID_INIT_PAYLOAD_SIZE : U2FHID_CONT_PAYLOAD_SIZE;
        xfer_data_len = (remaining < xfer_data_max) ? remaining : xfer_data_max;

        // パケットを生成
        generate_hid_input_report(send_info_t.payload_data, send_info_t.payload_length, i, xfer_data_len, send_info_t.cid, send_info_t.cmd);

        // パケットをU2Fクライアントへ転送
        usbd_hid_u2f_frame_send(hid_u2f_send_buffer, hid_u2f_send_buffer_length);
        break;
    }

    // 処理タイムアウト監視を停止
    hid_u2f_comm_interval_timer_stop();
}

void hid_u2f_send_input_report_complete()
{
    // 送信が正常に完了した時の処理
    //   hid_user_ev_handlerのイベント
    //   APP_USBD_HID_USER_EVT_IN_REPORT_DONEの
    //   発生でコールバックされる
    NRF_LOG_DEBUG("APP_USBD_HID_USER_EVT_IN_REPORT_DONE");
}
