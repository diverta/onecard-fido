/* 
 * File:   hid_fido_command.c
 * Author: makmorit
 *
 * Created on 2018/11/21, 14:21
 */
#include "sdk_common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fds.h"
#include "usbd_hid_comm_interval_timer.h"
#include "hid_fido_receive.h"
#include "hid_fido_send.h"
#include "hid_u2f_command.h"
#include "hid_ctap2_command.h"

// for U2F command
#include "u2f.h"
#include "ctap2.h"

// for lighting LED on/off
#include "fido_idling_led.h"

// for logging informations
#define NRF_LOG_MODULE_NAME hid_fido_command
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

static void send_error_command_response(uint8_t error_code) 
{
    // U2F ERRORコマンドに対応する
    // レスポンスデータを送信パケットに設定し送信
    uint32_t cid = hid_fido_receive_hid_header()->CID;
    hid_fido_send_error_command_response(cid, U2F_COMMAND_ERROR, error_code);

    // アイドル時点滅処理を開始
    fido_idling_led_on(LED_FOR_PROCESSING);
}

void hid_fido_command_on_report_received(uint8_t *request_frame_buffer, size_t request_frame_number)
{
    // 受信したフレームから、リクエストデータを取得し、
    // 同時に内容をチェックする
    hid_fido_receive_request_data(request_frame_buffer, request_frame_number);

    uint8_t cmd = hid_fido_receive_hid_header()->CMD;
    if (cmd == U2F_COMMAND_ERROR) {
        // チェック結果がNGの場合はここで処理中止
        send_error_command_response(hid_fido_receive_hid_header()->ERROR);
        return;
    }

    // データ受信後に実行すべき処理を判定
    switch (cmd) {
#if CTAP2_SUPPORTED
        case CTAP2_COMMAND_INIT:
            hid_ctap2_command_init();
            break;
#else
        case U2F_COMMAND_HID_INIT:
            hid_u2f_command_init();
            break;
#endif
        case U2F_COMMAND_MSG:
            hid_u2f_command_msg();
            break;
        case CTAP2_COMMAND_CBOR:
            hid_ctap2_command_cbor();
            break;
        default:
            break;
    }
}

void hid_fido_command_on_fs_evt(fds_evt_t const *const p_evt)
{
    // Flash ROM更新後に行われる後続処理を実行
    uint8_t cmd = hid_fido_receive_hid_header()->CMD;
    switch (cmd) {
        case U2F_COMMAND_MSG:
            hid_u2f_command_msg_send_response(p_evt);
            break;
        case CTAP2_COMMAND_CBOR:
            hid_ctap2_command_cbor_send_response(p_evt);
            break;
        default:
            break;
    }
}

void hid_fido_command_on_report_sent(void)
{
    // 全フレーム送信後に行われる後続処理を実行
    uint8_t cmd = hid_fido_receive_hid_header()->CMD;
    switch (cmd) {
        case U2F_COMMAND_MSG:
            hid_u2f_command_msg_report_sent();
            break;
        case CTAP2_COMMAND_CBOR:
            hid_ctap2_command_cbor_report_sent();
            break;
        default:
            break;
    }
}

void hid_fido_command_on_process_started(void) 
{
    // 処理タイムアウト監視を開始
    usbd_hid_comm_interval_timer_start();

    // アイドル時点滅処理を停止
    fido_idling_led_off(LED_FOR_PROCESSING);
}

void hid_fido_command_on_process_ended(void) 
{
    // 処理タイムアウト監視を停止
    usbd_hid_comm_interval_timer_stop();

    // アイドル時点滅処理を開始
    fido_idling_led_on(LED_FOR_PROCESSING);
}

void hid_fido_command_on_process_timedout(void) 
{
    // USBポートにタイムアウトを通知する
    NRF_LOG_ERROR("USB HID communication timed out.");
    
    // コマンドをU2F ERRORに変更のうえ、
    // レスポンスデータを送信パケットに設定し送信
    send_error_command_response(0x7f);
}

bool hid_fido_command_is_valid(uint8_t command)
{
    // FIDO機能（U2F、CTAP2）の
    // コマンドであればtrueを戻す
    switch (command) {
        // U2F関連コマンド
        case U2F_COMMAND_PING:
        case U2F_COMMAND_MSG:
        case U2F_COMMAND_HID_LOCK:
        case U2F_COMMAND_HID_INIT:
        case U2F_COMMAND_HID_WINK:

        // CTAP2関連コマンド
        case CTAP2_COMMAND_CBOR:
            return true;
        default:
            return false;
    }
}
