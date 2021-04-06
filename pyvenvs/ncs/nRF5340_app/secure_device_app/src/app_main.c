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

    // Bluetoothサービスを開始
    app_bluetooth_start();

    // 初期処理完了済み
    app_initialized = true;
}
