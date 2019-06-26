/* 
 * File:   fido_command.c
 * Author: makmorit
 *
 * Created on 2019/02/11, 11:31
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
//
// プラットフォーム非依存コード
//
#include "fido_u2f_command.h"
#include "fido_ctap2_command.h"
#include "fido_ble_command.h"
#include "fido_hid_command.h"
#include "fido_nfc_command.h"

// 移行中のモジュール
#include "ble_ctap2_command.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// for keepalive timer
#include "fido_timer.h"

// for fido_ble_pairing_change_mode
#include "fido_ble_peripheral.h"
#include "fido_ble_pairing.h"
#include "fido_ble_service.h"

void fido_command_on_mainsw_event(void)
{
    // ボタンが短押しされた時の処理を実行
    if (fido_ble_get_U2F_context()->conn_handle != BLE_CONN_HANDLE_INVALID) {
        if (fido_ble_command_on_mainsw_event() == true) {
            return;
        }
    }
    if (hid_u2f_command_on_mainsw_event() == true) {
        return;
    }
    fido_ctap2_command_on_mainsw_event();
    ble_ctap2_command_on_mainsw_event();
}

void fido_command_on_mainsw_long_push_event(void)
{
    // ボタンが長押しされた時の処理を実行
    if (fido_ble_peripheral_mode()) {
        // BLEペリフェラルが稼働時は、
        // ペアリングモード変更を実行
        fido_ble_pairing_change_mode();
    }
}

void fido_command_on_process_timedout(void) 
{
    // 処理タイムアウト発生時の処理を実行
    //
    // 処理中表示LEDが点滅していた場合は
    // ここでLEDを消灯させる
    fido_processing_led_off();

    // アイドル時点滅処理を再開
    fido_idling_led_on();
}

void fido_command_keepalive_timer_handler(void *p_context)
{
    // キープアライブ・コマンドを実行する
    if (p_context == NULL) {
        fido_ctap2_command_keepalive_timer_handler();
    } else {
        fido_ble_command_keepalive_timer_handler(p_context);
    }
}

void fido_user_presence_terminate(void)
{
    // タイマーを停止する
    fido_keepalive_interval_timer_stop();
}

void fido_user_presence_verify_start(uint32_t timeout_msec, void *p_context)
{
    // タイマーが生成されていない場合は生成する
    fido_keepalive_interval_timer_start(timeout_msec, p_context);

    // LED点滅を開始
    fido_processing_led_on(LED_LIGHT_FOR_USER_PRESENCE, LED_ON_OFF_INTERVAL_MSEC);
}

uint8_t fido_user_presence_verify_end(void)
{
    // LEDを消灯させる
    fido_processing_led_off();

    // タイマーを停止する
    fido_keepalive_interval_timer_stop();
    
    // User presence byte(0x01)を生成
    return 0x01;
}
