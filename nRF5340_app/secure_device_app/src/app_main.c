/* 
 * File:   app_main.c
 * Author: makmorit
 *
 * Created on 2021/04/02, 15:04
 */
#include <zephyr/types.h>
#include <zephyr/kernel.h>

#include "app_bluetooth.h"
#include "app_board.h"
#include "app_crypto.h"
#include "app_crypto_define.h"
#include "app_event.h"
#include "app_main.h"
#include "app_rtcc.h"
#include "app_timer.h"
#include "app_usb.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_main);

#define LOG_DEBUG_HID_DATA_FRAME    false
#define LOG_DEBUG_CCID_DATA_FRAME   false
#define LOG_DEBUG_BLE_DATA_FRAME    false

//
// 業務処理関連
//
#include "fido_ble_receive.h"
#include "fido_ble_send.h"
#include "fido_command.h"
#include "fido_hid_channel.h"
#include "fido_hid_receive.h"
#include "fido_hid_send.h"
#include "ctap2_client_pin.h"
#include "ccid.h"
#include "rtcc.h"

// for resume after Flash ROM object updated/deleted
#include "fido_flash.h"
#include "ccid_flash_object.h"

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

    // 暗号化関連の初期化
    // 処理完了後、Bluetoothサービス開始を指示
    //   同時に、Flash ROMストレージが
    //   使用可能となります。
    app_main_app_crypto_do_process(CRYPTO_EVT_INIT, app_bluetooth_start);
}

//
// データ処理イベント関連
//
static bool m_initialized = false;

void app_main_data_channel_initialized(void)
{
    // 初期化処理が実行済みの場合は終了
    if (m_initialized) {
        return;
    }

    // BLEまたはUSB HID I/Fが使用可能になった場合、
    // 業務処理をオープンさせる前に、
    // CTAP2で使用する機密データを初期化
    printk("ctap2_client_pin_init will skip... \n\r");
    //ctap2_client_pin_init();
    
    // RTCCを初期化
    rtcc_init();

    // バージョンをデバッグ出力
    LOG_INF("Secure device application (%s) version %s", CONFIG_BT_DIS_HW_REV_STR, CONFIG_BT_DIS_FW_REV_STR);

    // 初期化処理完了
    m_initialized = true;
}

bool app_main_is_data_channel_initialized(void)
{
    // 初期化処理が完了しているかどうかを戻す
    return m_initialized;
}

void app_main_hid_configured(void)
{
    // USB HID I/Fが使用可能になった場合、
    // アプリケーションで使用するCIDを初期化
    fido_hid_channel_initialize_cid();
}

void app_main_hid_data_frame_received(uint8_t *data, size_t size)
{
    if (fido_hid_receive_request_frame(data, size)) {
        app_event_notify(APEVT_HID_REQUEST_RECEIVED);
    }
#if LOG_DEBUG_HID_DATA_FRAME
    LOG_DBG("received %d bytes", size);
    LOG_HEXDUMP_DBG(data, size, "HID report");
#endif
}

void app_main_hid_request_received(void)
{
    fido_hid_receive_on_request_received();
}

void app_main_hid_report_sent(void)
{
    fido_hid_send_input_report_complete();
}

void app_main_ccid_data_frame_received(uint8_t *data, size_t size)
{
    if (ccid_data_frame_received(data, size)) {
        app_event_notify(APEVT_CCID_REQUEST_RECEIVED);
    }

#if LOG_DEBUG_CCID_DATA_FRAME
    LOG_DBG("received %d bytes", size);
    LOG_HEXDUMP_DBG(data, size, "CCID data");
#endif
}

void app_main_ccid_request_received(void)
{
    ccid_request_apdu_received();

#if LOG_DEBUG_CCID_DATA_FRAME
    LOG_DBG("CCID request received");
#endif
}

void app_main_ble_data_frame_received(uint8_t *data, size_t size)
{
    if (fido_ble_receive_control_point(data, size)) {
        app_event_notify(APEVT_BLE_REQUEST_RECEIVED);
    }
#if LOG_DEBUG_BLE_DATA_FRAME
    LOG_DBG("received %d bytes", size);
    LOG_HEXDUMP_DBG(data, size, "BLE data");
#endif
}

void app_main_ble_request_received(void)
{
    fido_ble_receive_on_request_received();
}

void app_main_ble_response_sent(void)
{
    fido_ble_send_on_tx_complete();
#if LOG_DEBUG_BLE_DATA_FRAME
    LOG_DBG("BLE data sent");
#endif
}

//
// Flash ROM更新時の処理
//
void app_main_app_settings_saved(void)
{
    fido_flash_object_record_updated();
    ccid_flash_object_record_updated();
}

void app_main_app_settings_deleted(void)
{
    fido_flash_object_record_deleted();
    ccid_flash_object_record_deleted();
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
    // 現在未割り当て
    LOG_INF("Button 2 pressed");
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
