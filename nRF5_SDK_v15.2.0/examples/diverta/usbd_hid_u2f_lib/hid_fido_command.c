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
#include "hid_u2f_receive.h"
#include "hid_u2f_send.h"
#include "hid_u2f_command.h"

// for ble_u2f_processing_led_on/off
#include "one_card_main.h"
#include "u2f_idling_led.h"

// for logging informations
#define NRF_LOG_MODULE_NAME hid_fido_command
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

static void send_error_command_response(uint8_t error_code) 
{
    // レスポンスデータを編集 (1 bytes)
    uint8_t err_response_buffer[1] = {error_code};
    size_t  err_response_length = sizeof(err_response_buffer); 

    // U2F ERRORコマンドに対応する
    // レスポンスデータを送信パケットに設定し送信
    uint32_t cid = hid_u2f_receive_hid_header()->CID;
    hid_u2f_send_setup(cid, U2F_COMMAND_ERROR, err_response_buffer, err_response_length);
    hid_u2f_send_input_report();

    // アイドル時点滅処理を開始
    u2f_idling_led_on(one_card_get_U2F_context()->led_for_processing_fido);
}

void hid_u2f_command_on_report_received(uint8_t *request_frame_buffer, size_t request_frame_number)
{
    // 受信したフレームから、リクエストデータを取得し、
    // 同時に内容をチェックする
    hid_u2f_receive_request_data(request_frame_buffer, request_frame_number);

    uint8_t cmd = hid_u2f_receive_hid_header()->CMD;
    if (cmd == U2F_COMMAND_ERROR) {
        // チェック結果がNGの場合はここで処理中止
        send_error_command_response(hid_u2f_receive_hid_header()->ERROR);
        return;
    }

    uint8_t ins;
    NRF_LOG_INFO("CMD(0x%02x) LEN(%d)", 
        hid_u2f_receive_hid_header()->CMD, 
        hid_u2f_receive_hid_header()->LEN);

    // データ受信後に実行すべき処理を判定
    switch (cmd) {
        case U2F_COMMAND_HID_INIT:
            u2f_hid_init_do_process();
            break;
            
        case U2F_COMMAND_MSG:
            // u2f_request_buffer の先頭バイトを参照
            //   [0]CLA [1]INS [2]P1 3[P2]
            ins = hid_u2f_receive_apdu()->INS;
            if (ins == U2F_VERSION) {
                u2f_version_do_process();
                
            } else if (ins == U2F_REGISTER) {
                u2f_register_do_process();
                
            } else if (ins == U2F_AUTHENTICATE) {
                u2f_authenticate_do_process();
            }
            break;
        default:
            break;
    }
}

void hid_u2f_command_on_fs_evt(fds_evt_t const *const p_evt)
{
    uint8_t  ins;

    // Flash ROM更新後に行われる後続処理を実行
    uint8_t cmd = hid_u2f_receive_hid_header()->CMD;
    switch (cmd) {
        case U2F_COMMAND_MSG:
            // u2f_request_buffer の先頭バイトを参照
            //   [0]CLA [1]INS [2]P1 3[P2]
            ins = hid_u2f_receive_apdu()->INS;
            if (ins == U2F_REGISTER) {
                u2f_register_send_response(p_evt);
                
            } else if (ins == U2F_AUTHENTICATE) {
                u2f_authenticate_send_response(p_evt);
            }
            break;
        default:
            break;
    }
}

void hid_u2f_command_on_report_sent(void)
{
    uint8_t  ins;

    // 全フレーム送信後に行われる後続処理を実行
    uint8_t cmd = hid_u2f_receive_hid_header()->CMD;
    switch (cmd) {
        case U2F_COMMAND_MSG:
            // u2f_request_buffer の先頭バイトを参照
            //   [0]CLA [1]INS [2]P1 3[P2]
            ins = hid_u2f_receive_apdu()->INS;
            if (ins == U2F_REGISTER) {
                NRF_LOG_INFO("U2F Register end");
                
            } else if (ins == U2F_AUTHENTICATE) {
                NRF_LOG_INFO("U2F Authenticate end");
            }
            break;
        default:
            break;
    }
}

void hid_u2f_command_on_process_started(void) 
{
    // 処理タイムアウト監視を開始
    usbd_hid_comm_interval_timer_start();

    // アイドル時点滅処理を停止
    u2f_idling_led_off(one_card_get_U2F_context()->led_for_processing_fido);
}

void hid_u2f_command_on_process_ended(void) 
{
    // 処理タイムアウト監視を停止
    usbd_hid_comm_interval_timer_stop();

    // アイドル時点滅処理を開始
    u2f_idling_led_on(one_card_get_U2F_context()->led_for_processing_fido);
}

void hid_u2f_command_on_process_timedout(void) 
{
    // USBポートにタイムアウトを通知する
    NRF_LOG_ERROR("USB HID communication timed out.");
    
    // コマンドをU2F ERRORに変更のうえ、
    // レスポンスデータを送信パケットに設定し送信
    send_error_command_response(0x7f);
}

bool hid_u2f_command_is_valid(uint8_t command)
{
    switch (command) {
        case U2F_COMMAND_PING:
        case U2F_COMMAND_MSG:
        case U2F_COMMAND_HID_LOCK:
        case U2F_COMMAND_HID_INIT:
        case U2F_COMMAND_HID_WINK:
            return true;
        default:
            return false;
    }
}
