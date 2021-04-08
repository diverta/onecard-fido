/* 
 * File:   app_main.c
 * Author: makmorit
 *
 * Created on 2021/04/02, 15:04
 */
#include <zephyr/types.h>
#include <zephyr.h>

#include "app_bluetooth.h"
#include "app_main.h"
#include "app_board.h"
#include "app_event.h"
#include "app_flash.h"
#include "app_timer.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_main);

//
// アプリケーション状態を保持
//
static bool app_initialized = false;

bool app_main_initialized(void)
{
    return app_initialized;
}

//
// 業務処理スレッド
//
void app_main_init(void) 
{
    // ボタン、LEDを使用可能にする
    app_board_initialize();

    // Flash ROMを使用可能にする
    app_flash_initialize();

    // タイマーを使用可能にする
    app_timer_initialize();
    
    // Bluetoothサービスを開始
    app_bluetooth_start();

    // 初期処理完了済み
    app_initialized = true;
}

static void app_main_thread(void)
{
    while (true) {
        // 各種イベントを処理
        app_event_process();
    }
}

// STACKSIZE: size of stack area used by thread
// PRIORITY:  scheduling priority used by thread
#define STACKSIZE   1024
#define PRIORITY    7
K_THREAD_DEFINE(app_main_thread_id, STACKSIZE, app_main_thread, NULL, NULL, NULL, PRIORITY, 0, 0);
