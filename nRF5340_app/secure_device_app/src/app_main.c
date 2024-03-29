/* 
 * File:   app_main.c
 * Author: makmorit
 *
 * Created on 2021/04/02, 15:04
 */
#include "app_ble_init.h"
#include "app_board.h"
#include "app_crypto.h"
#include "app_crypto_define.h"
#include "app_event.h"
#include "app_main.h"
#include "app_rtcc.h"
#include "app_timer.h"
#include "app_usb.h"

//
// アプリケーション初期化処理
//
void app_main_init(void) 
{
    // ボタン、LEDを使用可能にする
    app_board_initialize();

    // USBを使用可能にする
    app_usb_initialize();

    // タイマーを使用可能にする
    app_timer_initialize();

    // 業務処理イベント（APEVT_XXXX）を
    // 通知できるようにする
    app_event_main_enable(true);

    // サブシステム初期化をメインスレッドで実行
    app_event_notify(APEVT_SUBSYS_INIT);
}

void app_main_subsys_init(void)
{
    // リアルタイムクロックカレンダーの初期化
    app_rtcc_initialize();

    // 暗号化関連の初期化
    //   別スレッドでランダムシードを生成
    app_crypto_do_process(CRYPTO_EVT_INIT);
}

void app_main_app_crypto_init_done(void)
{
    // 暗号化関連の初期化処理完了
    //   Bluetoothサービス開始を指示
    //   同時に、Flash ROMストレージが
    //   使用可能となります。
    app_ble_init();
}

//
// 暗号化関連処理
//
static void (*_resume_func)(void);

void app_main_app_crypto_do_process(uint8_t event, void (*resume_func)(void))
{
    // コールバック関数の参照を保持
    _resume_func = resume_func;

    // 暗号化関連処理を専用スレッドで実行
    app_crypto_do_process(event);
}

void app_main_app_crypto_done(void)
{
    // コールバック関数を実行
    if (_resume_func != NULL) {
        (*_resume_func)();
        _resume_func = NULL;
    }
}
