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

// レスポンス完了後の処理を停止させるフラグ
static bool abort_flag = false;

bool fido_command_do_abort(void)
{
    // レスポンス完了後の処理を停止させる場合は、
    // 全色LEDを点灯させたのち、無限ループに入る
    if (abort_flag) {
        fido_status_indicator_abort();
        while(true);
    }
    return abort_flag;
}

void fido_command_abort_flag_set(bool flag)
{
    abort_flag = flag;
}

void fido_command_mainsw_event_handler(void)
{
    // ボタンが短押しされた時の処理を実行
    if (fido_u2f_command_on_mainsw_event() == true) {
        return;
    }
    fido_ctap2_command_on_mainsw_event();
}

void fido_command_process_timeout_handler(void) 
{
    // 処理タイムアウト発生時の処理を実行
    //
    // ユーザー所在確認が未だ完了していない場合、
    // ここでキャンセルさせる
    fido_u2f_command_tup_cancel();
    fido_ctap2_command_tup_cancel();

    // LED制御をアイドル中（秒間２回点滅）に変更
    fido_status_indicator_idle();
}

void fido_command_keepalive_timer_handler(void)
{
    // キープアライブ・コマンドを実行する
    fido_ctap2_command_keepalive_timer_handler();
    fido_u2f_command_keepalive_timer_handler();
}

void fido_user_presence_verify_start(uint32_t timeout_msec)
{
    // タイマーが生成されていない場合は生成する
    fido_keepalive_interval_timer_start(timeout_msec, NULL);

    // LED点滅を開始
    fido_status_indicator_prompt_tup();
}

void fido_user_presence_verify_end(void)
{
    // LED制御をユーザー所在確認中-->非アイドル中に変更
    fido_status_indicator_no_idle();

    // タイマーを停止する
    fido_keepalive_interval_timer_stop();
}

void fido_user_presence_verify_cancel(void)
{
    // タイマーを停止する
    fido_keepalive_interval_timer_stop();
}
