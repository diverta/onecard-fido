/* 
 * File:   fido_timer.c
 * Author: makmorit
 *
 * Created on 2021/09/21, 11:43
 */
//
// プラットフォーム非依存コード
//
#include "fido_command.h"
#include "fido_hid_channel.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// ユーザー所在確認タイムアウト監視用タイマー（３０秒）
//
void fido_user_presence_verify_timer_stop(void)
{
    // 業務処理用 汎用ワンショットタイマーを停止させる
    app_timer_stop_for_generic_oneshot();
}

void fido_user_presence_verify_timer_start(uint32_t timeout_msec, void *p_context)
{
    // 業務処理用 汎用ワンショットタイマーを開始させる
    (void)p_context;
    app_timer_start_for_generic_oneshot(timeout_msec, fido_user_presence_verify_timeout_handler);
}

//
// 反復処理汎用タイマー
// 
void fido_repeat_process_timer_stop(void)
{
    // 業務処理用 汎用リピートタイマーを停止する
    app_timer_stop_for_generic_repeat();
}

void fido_repeat_process_timer_start(uint32_t timeout_msec, void (*handler)(void))
{
    // 業務処理用 汎用リピートタイマーを開始させる
    app_timer_start_for_generic_repeat(timeout_msec, handler);
}

//
// HIDチャネルロック用タイマー
// 
void fido_hid_channel_lock_timer_stop(void)
{
    // 業務処理用 汎用ワンショットタイマーを停止させる
    app_timer_stop_for_generic_oneshot();
}

void fido_hid_channel_lock_timer_start(uint32_t lock_ms)
{
    // 業務処理用 汎用ワンショットタイマーを開始させる
    app_timer_start_for_generic_oneshot(lock_ms, fido_hid_channel_lock_timedout_handler);
}
