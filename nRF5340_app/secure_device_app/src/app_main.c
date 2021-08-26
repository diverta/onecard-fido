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
#include "app_process.h"
#include "app_timer.h"
#include "app_usb.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_main);

#define LOG_DEBUG_HID_REPORT        false
#define LOG_DEBUG_CCID_DATA         false

//
// 業務処理関連
//
#include "fido_hid_channel.h"
#include "fido_hid_receive.h"
#include "fido_hid_send.h"

//
// アプリケーション状態を保持
//
static bool app_initialized = false;

bool app_main_initialized(void)
{
    return app_initialized;
}

//
// アプリケーション初期化処理
//
void app_main_init(void) 
{
    // ボタン、LEDを使用可能にする
    app_board_initialize();

    // USBを使用可能にする
    app_usb_initialize();

    // Flash ROMを使用可能にする
    app_flash_initialize();

    // タイマーを使用可能にする
    app_timer_initialize();
    
    // Bluetoothサービスを開始
    app_bluetooth_start();

    // その他の初期化処理
    app_process_initialize();

    // 業務処理の初期化
    // アプリケーションで使用するCIDを初期化
    fido_hid_channel_initialize_cid();

    // 初期処理完了済み
    app_initialized = true;
}

//
// データ処理イベント関連
//
void app_main_hid_report_received(uint8_t *data, size_t size)
{
    if (fido_hid_receive_request_frame(data, size)) {
        fido_hid_receive_on_request_received();
    }
#if LOG_DEBUG_HID_REPORT
    LOG_DBG("received %d bytes", size);
    LOG_HEXDUMP_DBG(data, size, "HID report");
#endif
}

void app_main_hid_report_sent(void)
{
    fido_hid_send_input_report_complete();
}

void app_main_ccid_data_received(uint8_t *data, size_t size)
{
#if LOG_DEBUG_CCID_DATA
    LOG_DBG("received %d bytes", size);
    LOG_HEXDUMP_DBG(data, size, "CCID data");
#endif
}

//
// ボタン押下時の処理
//
void app_main_button_pressed_short(void)
{
    LOG_DBG("Short pushed");
}

void app_main_button_1_pressed(void)
{
    LOG_DBG("Button 2 pushed");
}
