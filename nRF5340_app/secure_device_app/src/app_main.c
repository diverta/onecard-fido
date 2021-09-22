/* 
 * File:   app_main.c
 * Author: makmorit
 *
 * Created on 2021/04/02, 15:04
 */
#include <zephyr/types.h>
#include <zephyr.h>

#include "app_bluetooth.h"
#include "app_board.h"
#include "app_event.h"
#include "app_timer.h"
#include "app_usb.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_main);

#define LOG_DEBUG_HID_REPORT        false
#define LOG_DEBUG_CCID_DATA         false
#define LOG_DEBUG_BLE_DATA          false

//
// 業務処理関連
//
#include "fido_ble_receive.h"
#include "fido_ble_send.h"
#include "fido_command.h"
#include "fido_hid_channel.h"
#include "fido_hid_receive.h"
#include "fido_hid_send.h"

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

    // Bluetoothサービス開始を指示
    //   同時に、Flash ROMストレージが
    //   使用可能となります。
    app_bluetooth_start();
}

//
// データ処理イベント関連
//
void app_main_hid_configured(void)
{
    // USB HID I/Fが使用可能になった場合、
    // アプリケーションで使用するCIDを初期化
    fido_hid_channel_initialize_cid();
}

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

void app_main_ble_request_received(uint8_t *data, size_t size)
{
    if (fido_ble_receive_control_point(data, size)) {
        fido_ble_receive_on_request_received();
    }
#if LOG_DEBUG_BLE_DATA
    LOG_DBG("received %d bytes", size);
    LOG_HEXDUMP_DBG(data, size, "BLE data");
#endif
}

void app_main_ble_response_sent(void)
{
    fido_ble_send_on_tx_complete();
#if LOG_DEBUG_BLE_DATA
    LOG_DBG("BLE data sent");
#endif
}

//
// ボタン押下時の処理
//
void app_main_button_pressed_short(void)
{
    // FIDO固有の処理を実行
    fido_command_mainsw_event_handler();
}

void app_main_button_1_pressed(void)
{
    LOG_DBG("Button 2 pushed");
}
