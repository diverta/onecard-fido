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

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// for keepalive timer
#include "fido_timer.h"

// for fido_ble_pairing_change_mode
#include "fido_ble_peripheral.h"
#include "fido_ble_pairing.h"
#include "fido_ble_service.h"

// レスポンス完了後の処理を停止させるフラグ
static bool abort_flag = false;

bool fido_command_do_abort(void)
{
    // レスポンス完了後の処理を停止させる場合は、
    // 全色LEDを点灯させたのち、無限ループに入る
    if (abort_flag) {
        fido_led_light_all(true);
        while(true);
    }
    return abort_flag;
}

void fido_command_abort_flag_set(bool flag)
{
    abort_flag = flag;
}

void fido_command_on_mainsw_event(void)
{
    // ボタンが短押しされた時の処理を実行
    if (fido_u2f_command_on_mainsw_event() == true) {
        return;
    }
    fido_ctap2_command_on_mainsw_event();
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
    // fido_led_blink_stop();

    // アイドル時点滅処理を再開
    fido_idling_led_blink_start();
}

void fido_command_keepalive_timer_handler(void)
{
    // キープアライブ・コマンドを実行する
    fido_ctap2_command_keepalive_timer_handler();
    fido_u2f_command_keepalive_timer_handler();
}

void fido_user_presence_terminate(void)
{
    // タイマーを停止する
    fido_keepalive_interval_timer_stop();
}

void fido_user_presence_verify_start(uint32_t timeout_msec)
{
    // タイマーが生成されていない場合は生成する
    fido_keepalive_interval_timer_start(timeout_msec);

    // LED点滅を開始
    fido_prompt_led_blink_start(LED_ON_OFF_INTERVAL_MSEC);
}

uint8_t fido_user_presence_verify_end(void)
{
    // LEDを消灯させる
    fido_led_blink_stop();

    // タイマーを停止する
    fido_keepalive_interval_timer_stop();
    
    // User presence byte(0x01)を生成
    return 0x01;
}
