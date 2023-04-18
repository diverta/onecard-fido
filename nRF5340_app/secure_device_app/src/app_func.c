/* 
 * File:   app_func.c
 * Author: makmorit
 *
 * Created on 2023/02/22, 14:08
 */
#include <zephyr/types.h>
#include <zephyr/kernel.h>

#include "app_event.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_func);

#define APP_NO_FUNCTION             false
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
#include "fido_platform.h"
#include "rtcc.h"

// for resume after Flash ROM object updated/deleted
#include "fido_flash.h"
#include "ccid_flash_object.h"

//
// データ処理イベント関連
//
static bool m_initialized = false;

bool app_main_is_data_channel_initialized(void)
{
    // 初期化処理が完了しているかどうかを戻す
    return m_initialized;
}

#if !APP_NO_FUNCTION
static void crypto_random_pre_generated(void);

void app_main_data_channel_initialized(void)
{
    // 初期化処理が実行済みの場合は終了
    if (m_initialized) {
        return;
    }

    // `ctap2_client_pin_init`内で実行される
    // `fido_command_generate_random_vector`の実行事前に、
    // ランダムベクターの生成を指示
    fido_crypto_random_pre_generate(crypto_random_pre_generated);
}    

static void crypto_random_pre_generated(void)
{
    // BLEまたはUSB HID I/Fが使用可能になった場合、
    // 業務処理をオープンさせる前に、
    // CTAP2で使用する機密データを初期化
    ctap2_client_pin_init();

    // RTCCを初期化
    rtcc_init();

    // TFTディスプレイを初期化
    tiny_tft_init_display();

    // バージョンをデバッグ出力
    LOG_INF("Secure device application (%s) version %s (%d)", CONFIG_BT_DIS_HW_REV_STR, CONFIG_BT_DIS_FW_REV_STR, CONFIG_APP_FW_BUILD);

    // 初期化処理完了
    m_initialized = true;
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

#else
//
// プラットフォーム固有の障害切り分け時には、
// 全ての業務コードをビルド対象から外し、
// 以下のブロックを有効化します。
//
void app_main_data_channel_initialized(void)
{
    // 初期化処理が実行済みの場合は終了
    if (m_initialized) {
        return;
    }

    // RTCCを初期化
    rtcc_init();

    // バージョンをデバッグ出力
    LOG_INF("Secure device application with no function (%s) version %s", CONFIG_BT_DIS_HW_REV_STR, CONFIG_BT_DIS_FW_REV_STR);

    // 初期化処理完了
    m_initialized = true;
}

void app_main_hid_configured(void)
{
}

void app_main_hid_data_frame_received(uint8_t *data, size_t size)
{
}

void app_main_hid_request_received(void)
{
}

void app_main_hid_report_sent(void)
{
}

void app_main_ccid_data_frame_received(uint8_t *data, size_t size)
{
}

void app_main_ccid_request_received(void)
{
}

void app_main_ble_data_frame_received(uint8_t *data, size_t size)
{
}

void app_main_ble_request_received(void)
{
}

void app_main_ble_response_sent(void)
{
}

void app_main_app_settings_saved(void)
{
}

void app_main_app_settings_deleted(void)
{
}

void app_main_button_pressed_short(void)
{
    LOG_INF("Button 1 pressed");
}

void app_main_button_1_pressed(void)
{
    LOG_INF("Button 2 pressed");
}
#endif //#if APP_NO_FUNCTION
