/* 
 * File:   fido_hid_send.c
 * Author: makmorit
 *
 * Created on 2018/11/21, 14:21
 */
#include <stdio.h>
#include <string.h>
//
// プラットフォーム非依存コード
//
#include "ctap2_common.h"       // for CTAP2_COMMAND_KEEPALIVE
#include "fido_command.h"
#include "fido_hid_channel.h"
#include "fido_hid_receive.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

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


static void dump_hid_init_packet(USB_HID_MSG_T *recv_msg)
{
    // キープアライブコマンドの場合はログを出力しないようにする
    uint8_t *cid = recv_msg->cid;
    uint8_t  cmd = recv_msg->pkt.init.cmd;
    if (cmd == CTAP2_COMMAND_KEEPALIVE) {
        return;
    }

    size_t len = fido_hid_payload_length_get(recv_msg);
    if (cmd == CTAP2_COMMAND_INIT || cmd == CTAP2_COMMAND_PING) {
        fido_log_debug("INIT frame: CID(0x%08x) CMD(0x%02x) LEN(%d)",
            fido_hid_channel_get_cid_from_bytes(cid), cmd, len);
    } else {
        // レスポンスの先頭１バイト目＝ステータスコードである場合を想定したログ
        fido_log_debug("INIT frame: CID(0x%08x) CMD(0x%02x) LEN(%d) STATUS(0x%02x)",
            fido_hid_channel_get_cid_from_bytes(cid), cmd, len, recv_msg->pkt.init.payload[0]);
    }
}

static void dump_hid_cont_packet(USB_HID_MSG_T *recv_msg)
{
    fido_log_debug("CONT frame: CID(0x%08x) SEQ(0x%02x)",
        fido_hid_channel_get_cid_from_bytes(recv_msg->cid), recv_msg->pkt.cont.seq);
}

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
        fido_hid_channel_set_cid_bytes(res->cid, cid);
        res->pkt.init.cmd   = cmd;
        res->pkt.init.bcnth = (payload_length >> 8) & 0x00ff;
        res->pkt.init.bcntl = payload_length & 0x00ff;

        // パケットデータを設定
        if (payload_data != NULL) {
            memcpy(res->pkt.init.payload, payload_data + offset, xfer_data_len);
        }
        dump_hid_init_packet(res);

        // シーケンスを初期化
        seq = 0;

    } else {
        // チャネルID、シーケンスを設定
        fido_hid_channel_set_cid_bytes(res->cid, cid);
        res->pkt.cont.seq = seq++;

        // パケットデータを設定
        memcpy(res->pkt.cont.payload, payload_data + offset, xfer_data_len);
        dump_hid_cont_packet(res);
    }
}

static void hid_fido_send_setup(uint32_t cid, uint8_t cmd, uint8_t *payload_data, size_t payload_length)
{
    // 送信のために必要な情報を保持
    send_info_t.cid = cid;
    send_info_t.cmd = cmd;
    send_info_t.payload_data = payload_data;
    send_info_t.payload_length = payload_length;

    // 送信済みバイト数をゼロクリア
    send_info_t.sent_length = 0;
}

//
// hid_fido_send_input_report実行後に、
// hid_fido_send_input_report_completeを
// 実行しないかどうかを保持するフラグ
// 
static bool no_callback_input_report_complete;

static void hid_fido_send_input_report(bool no_callback)
{
    size_t  xfer_data_max;
    size_t  xfer_data_len;
    size_t  remaining;

    // 保持中の情報をチェック
    if (send_info_t.payload_length == 0 && send_info_t.payload_data == NULL) {
        // データ長＝０、データ＝NULLと明示的に指定された場合は
        // payloadなしのレスポンスを行う
    } else if (send_info_t.payload_length == 0 || send_info_t.payload_data == NULL) {
        // 完備していない場合は異常終了
        fido_log_error("hid_fido_send_input_report: hid_fido_send_setup incomplete ");
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
    
    // フラグを退避
    no_callback_input_report_complete = no_callback;
}

void fido_hid_send_input_report_complete(void)
{
    if (no_callback_input_report_complete) {
        // 実行する必要がない場合はスキップ
        return;
    }

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
        // FIDOレスポンス送信完了時の処理を実行
        fido_command_on_response_send_completed(TRANSPORT_HID);
        
    } else {
        // 次のフレームの送信を実行
        hid_fido_send_input_report(no_callback_input_report_complete);
    }
}

void fido_hid_send_command_response(uint32_t cid, uint8_t cmd, uint8_t *response_buffer, size_t response_length)
{
    hid_fido_send_setup(cid, cmd, response_buffer, response_length);
    hid_fido_send_input_report(false);
}

void fido_hid_send_command_response_no_payload(uint32_t cid, uint8_t cmd)
{
    hid_fido_send_setup(cid, cmd, NULL, 0);
    hid_fido_send_input_report(false);
}

static void fido_hid_send_command_response_no_callback(uint32_t cid, uint8_t cmd, uint8_t status_code) 
{
    // レスポンスデータを編集 (1 bytes)
    uint8_t cmd_response_buffer[1] = {status_code};
    size_t  cmd_response_length = sizeof(cmd_response_buffer); 

    // FIDO ERRORコマンドに対応する
    // レスポンスデータを送信パケットに設定し送信
    hid_fido_send_setup(cid, cmd, cmd_response_buffer, cmd_response_length);
    hid_fido_send_input_report(true);
}

void fido_hid_send_status_response(uint8_t cmd, uint8_t status_code) 
{
    // U2F ERRORコマンドに対応する
    // レスポンスデータを送信パケットに設定し送信
    uint32_t cid = fido_hid_receive_header()->CID;
    fido_hid_send_command_response_no_callback(cid, cmd, status_code);
}
