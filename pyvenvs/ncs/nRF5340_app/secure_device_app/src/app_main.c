/* 
 * File:   app_main.c
 * Author: makmorit
 *
 * Created on 2021/04/02, 15:04
 */
#include <zephyr/types.h>
#include <zephyr.h>

//
// for Bluetooth smp service
//
#include "os_mgmt/os_mgmt.h"
#include "img_mgmt/img_mgmt.h"

#include "app_main.h"
#include "app_board.h"

static bool app_initialized = false;

bool app_main_initialized(void)
{
    return app_initialized;
}

//
// 業務処理エントリー関数
//
void app_main(void) 
{
    // ボタン、LEDを使用可能にする
    app_board_initialize();

    // BLE SMPサービスの設定
    os_mgmt_register_group();
    img_mgmt_register_group();

    // Bluetoothサービスを開始
    app_bluetooth_start();

    // 初期処理完了済み
    app_initialized = true;
}
