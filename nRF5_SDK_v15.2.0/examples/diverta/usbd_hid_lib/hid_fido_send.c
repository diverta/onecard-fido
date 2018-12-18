/* 
 * File:   hid_fido_send.c
 * Author: makmorit
 *
 * Created on 2018/11/21, 14:21
 */
#include <stdio.h>

#include "usbd_hid_common.h"
#include "usbd_hid_service.h"
#include "hid_fido_command.h"

// for logging informations
#define NRF_LOG_MODULE_NAME hid_fido_send
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// FIDO機能のHIDリクエストデータ格納領域
static uint8_t hid_fido_send_buffer[USBD_HID_PACKET_SIZE];
static size_t  hid_fido_send_buffer_length;

static struct {
    // 送信用ヘッダーに格納するコマンド、データ長、
    // 送信データを保持
    uint32_t cid;
    uint8_t  cmd;
    uint8_t *payload_data;
    size_t   payload_length;
    // 送信済みバイト数を保持
    size_t   sent_length;
} send_info_t;

static void generate_hid_input_report(uint8_t *payload_data, size_t payload_length, 
                               size_t offset, size_t xfer_data_len, 
                               uint32_t cid, uint8_t cmd)
{
    static uint8_t seq;

    // 送信パケット格納領域を初期化
    memset(&hid_fido_send_buffer, 0x00, sizeof(hid_fido_send_buffer));
    hid_fido_send_buffer_length = USBD_HID_PACKET_SIZE;

    // パケット格納領域を取得
    USB_HID_MSG_T *res = (USB_HID_MSG_T *)hid_fido_send_buffer;

    if (offset == 0) {
        // チャネルID、CMD、データ長を設定
        set_CID(res->cid, cid);
        res->pkt.init.cmd   = cmd;
        res->pkt.init.bcnth = (payload_length >> 8) & 0x00ff;
        res->pkt.init.bcntl = payload_length & 0x00ff;

        // パケットデータを設定
        memcpy(res->pkt.init.payload, payload_data + offset, xfer_data_len);
        dump_hid_init_packet("Send ", hid_fido_send_buffer_length, res);

        // シーケンスを初期化
        seq = 0;

    } else {
        // チャネルID、シーケンスを設定
        set_CID(res->cid, cid);
        res->pkt.cont.seq = seq++;

        // パケットデータを設定
        memcpy(res->pkt.cont.payload, payload_data + offset, xfer_data_len);
        dump_hid_cont_packet("Send ", hid_fido_send_buffer_length, res);
    }
}

void hid_fido_send_setup(uint32_t cid, uint8_t cmd, uint8_t *payload_data, size_t payload_length)
{
    // 送信のために必要な情報を保持
    send_info_t.cid = cid;
    send_info_t.cmd = cmd;
    send_info_t.payload_data = payload_data;
    send_info_t.payload_length = payload_length;

    // 送信済みバイト数をゼロクリア
    send_info_t.sent_length = 0;
}

void hid_fido_send_input_report(void)
{
    size_t  xfer_data_max;
    size_t  xfer_data_len;
    size_t  remaining;

    // 保持中の情報をチェックし、
    // 完備していない場合は異常終了
    if (send_info_t.payload_length == 0 || send_info_t.payload_data == NULL) {
        NRF_LOG_ERROR("hid_fido_send_input_report: hid_fido_send_setup incomplete ");
        return;
    }
    
    // データ長
    remaining = send_info_t.payload_length - send_info_t.sent_length;
    xfer_data_max = (send_info_t.sent_length == 0) ? USBD_HID_INIT_PAYLOAD_SIZE : USBD_HID_CONT_PAYLOAD_SIZE;
    xfer_data_len = (remaining < xfer_data_max) ? remaining : xfer_data_max;

    // パケットを生成
    generate_hid_input_report(send_info_t.payload_data, send_info_t.payload_length, send_info_t.sent_length, xfer_data_len, send_info_t.cid, send_info_t.cmd);

    // パケットをFIDOクライアントへ転送
    usbd_hid_frame_send(hid_fido_send_buffer, hid_fido_send_buffer_length);

    // 送信済みバイト数を更新
    send_info_t.sent_length += xfer_data_len;
}

void hid_fido_send_input_report_complete()
{
    // hid_fido_send_input_report による
    // フレームの送信が正常に完了した時の処理
    //   hid_user_ev_handlerのイベント
    //   APP_USBD_HID_USER_EVT_IN_REPORT_DONEの
    //   発生でコールバックされる
    // 
    // 最終レコードの場合
    if (send_info_t.sent_length == send_info_t.payload_length) {
        // 送信情報を初期化
        memset(&send_info_t, 0x00, sizeof(send_info_t));
        // コールバック
        hid_fido_command_on_report_sent();
        // FIDO機能処理完了時の処理を実行
        hid_fido_command_on_process_ended();
        
    } else {
        // 次のフレームの送信を実行
        hid_fido_send_input_report();
    }
}
