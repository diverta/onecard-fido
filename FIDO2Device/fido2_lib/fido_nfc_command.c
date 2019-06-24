/* 
 * File:   fido_nfc_command.c
 * Author: makmorit
 *
 * Created on 2019/06/03, 15:20
 */
//
// プラットフォーム非依存コード
//
#include "fido_ctap2_command.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// プラットフォーム依存コード
// ターゲットごとの実装となります。
//
#include "fido_flash_event.h"   // for Flash ROM event
#include "fido_timer.h"         // for communication interval timer
#include "fido_nfc_receive.h"

void fido_nfc_command_on_fs_evt(fido_flash_event_t const *const p_evt)
{
    // Flash ROM更新完了時の処理を実行
    uint8_t cmd = fido_nfc_receive_apdu()->INS;
    switch (cmd) {
        case 0x10:
            // NFC CTAP2 command
            fido_ctap2_command_cbor_send_response(p_evt);
            break;
        default:
            break;
    }
}

void fido_nfc_command_on_send_completed(void)
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

void fido_nfc_command_on_request_started(void) 
{
    // FIDO機能リクエストの
    // 先頭フレーム受信時の処理を実行
    // 
    // 処理タイムアウト監視を開始
    fido_comm_interval_timer_start();

    // アイドル時点滅処理を停止
    fido_idling_led_off();
}
