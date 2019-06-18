/* 
 * File:   nfc_fido_command.c
 * Author: makmorit
 *
 * Created on 2019/06/03, 15:20
 */
#include "sdk_common.h"
#include "fds.h"

#include "fido_ctap2_command.h"
#include "nfc_fido_receive.h"

// for communication interval timer
#include "fido_timer.h"

// for lighting LED on/off
#include "fido_idling_led.h"

// for logging informations
#define NRF_LOG_MODULE_NAME nfc_fido_command
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

void nfc_fido_command_on_fs_evt(fds_evt_t const *const p_evt)
{
    // Flash ROM更新完了時の処理を実行
    uint8_t cmd = nfc_fido_receive_apdu()->INS;
    switch (cmd) {
        case 0x10:
            // NFC CTAP2 command
            fido_ctap2_command_cbor_send_response(p_evt);
            break;
        default:
            break;
    }
}

void nfc_fido_command_on_send_completed(void)
{
    // FIDO機能レスポンスの
    // 全フレーム送信完了時の処理を実行
    //
    // 処理タイムアウト監視を停止
    fido_comm_interval_timer_stop();

    // 全フレーム送信後に行われる後続処理を実行
    fido_ctap2_command_cbor_response_completed();

    // アイドル時点滅処理を開始
    fido_idling_led_on();
}

void nfc_fido_command_on_request_started(void) 
{
    // FIDO機能リクエストの
    // 先頭フレーム受信時の処理を実行
    // 
    // 処理タイムアウト監視を開始
    fido_comm_interval_timer_start();

    // アイドル時点滅処理を停止
    fido_idling_led_off();
}
