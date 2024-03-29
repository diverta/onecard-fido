/* 
 * File:   app_process.c
 * Author: makmorit
 *
 * Created on 2021/04/28, 10:22
 */
#include <zephyr/types.h>
#include <zephyr/kernel.h>

#include "app_ble_advertise.h"
#include "app_ble_pairing.h"
#include "app_board.h"
#include "app_event.h"
#include "app_flash_general_status.h"
#include "app_func.h"
#include "app_main.h"
#include "app_status_indicator.h"
#include "app_settings.h"
#include "app_timer.h"
#include "fido_platform.h"

// ログ出力制御
#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_process);

// ペアリング処理中かどうかを保持
static bool is_pairing_process = false;

void app_process_set_pairing_process_flag(bool b)
{
    is_pairing_process = b;
}

//
// ペアリングモード変更処理
//
static void initialize_pairing_mode(void)
{
    // ペアリングモード初期設定
    app_ble_pairing_mode_initialize();

    // BLEアドバタイズ開始を指示
    app_ble_advertise_start();

    // LED点灯パターン設定
    if (app_ble_pairing_mode()) {
        // ペアリングモード時は黄色LEDを連続点灯させる
        app_status_indicator_pairing_mode();
    } else {
        // アイドル時のLED点滅パターンを設定
        app_status_indicator_idle();
    }
}

static void change_to_pairing_mode(void)
{
    // ペアリングモード遷移-->アドバタイズ再開
    if (app_ble_pairing_mode_set(true)) {
        app_ble_advertise_start();
    }
}

//
// ボタンイベント振分け処理
//
static void button_pushed_long(void)
{
    // ボタン押下後、３秒経過した時の処理
    if (app_ble_pairing_mode() == false) {
        // 非ペアリングモード時は、
        // ペアリングモード遷移前に
        // 黄色LEDを連続点灯させる
        app_status_indicator_pairing_mode();
    }
}

static void button_pressed_long(void)
{
    // ボタン押下-->３秒経過後にボタンを離した時の処理
    LOG_DBG("Long pushed");
    if (app_ble_pairing_mode() == false) {
        // 非ペアリングモード時は、
        // ペアリングモード遷移-->アドバタイズ再開
        change_to_pairing_mode();
    }
}

static void button_pressed_short(void)
{
    // ボタン押下-->３秒以内にボタンを離した時の処理
    // 各種業務処理を実行
    if (app_main_button_pressed_short() == false) {
        if (app_ble_advertise_is_available()) {
            // BLEペリフェラルモードの場合
            if (is_pairing_process) {
                // ペアリング処理中はボタン押下を無視
                return;
            }
            if (app_ble_advertise_is_stopped()) {
                // ペアリング障害時にアドバタイズが停止された場合は
                // ボタン短押しでペアリングモードに遷移-->アドバタイズ再開
                change_to_pairing_mode();
                // 黄色LEDを連続点灯させる
                app_status_indicator_pairing_mode();
            } else {
                // ボタン短押しでスリープ状態に遷移
                app_event_notify(APEVT_IDLING_DETECTED);
            }
        }
    }
}

static void button_pressed(APP_EVENT_T event)
{
    // ボタン検知時刻を取得
    static uint32_t time_pressed = 0;
    uint32_t time_now = app_board_kernel_uptime_ms_get();

    // ボタン検知間隔を取得
    uint32_t elapsed = time_now - time_pressed;
    time_pressed = time_now;

    // ボタン検知間隔で判定
    if (event == APEVT_BUTTON_RELEASED) {
        if (elapsed > 3000) {
            // 長押し
            button_pressed_long();
        } else {
            // 短押し
            button_pressed_short();
        }
        // 開始済みのタイマーを停止
        app_timer_stop_for_longpush();
    }

    if (event == APEVT_BUTTON_PUSHED) {
        // ボタン長押し時に先行してLEDを
        // 点灯させるためのタイマーを開始
        app_timer_start_for_longpush(3000, APEVT_BUTTON_PUSHED_LONG);
    }
}

static void button_1_pressed(void)
{
    app_main_button_1_pressed();
}

static void idling_timer_start(void)
{
    // BLE接続アイドルタイマーを停止
    static bool timer_started = false;
    if (timer_started) {
        timer_started = false;
        app_timer_stop_for_idling();
    }

    // BLE接続アイドルタイマーを開始
    //   タイムアウト＝３分（ペアリングモード時＝90秒）
    uint32_t timeout = app_ble_pairing_mode() ? (CONFIG_BT_LIM_ADV_TIMEOUT * 1000) : 180000;
    app_timer_start_for_idling(timeout, APEVT_IDLING_DETECTED);
    timer_started = true;
}

//
// データチャネル関連処理
//
static void data_channel_initialized(void)
{
    // 業務処理の初期化
    app_main_data_channel_initialized();

    // データ処理イベント（DATEVT_XXXX）を
    // 通知できるようにする
    app_event_data_enable(true);

    // ボタン押下検知ができるようにする
    app_board_button_press_enable(true);
}

static void ble_available(void)
{
    // LED点滅管理用のタイマーを
    // 500ms後に始動させるようにする
    //   500ms wait --> 
    //   APEVT_LED_BLINK_BEGINが通知される
    app_timer_start_for_blinking(500, APEVT_LED_BLINK_BEGIN);

    // 永続化機能を初期化
    app_settings_initialize();
}

static void ble_unavailable(void)
{
    // 全色LEDを点灯
    app_status_indicator_abort();
}

static void ble_advertise_started(void)
{
    // BLE接続アイドルタイマーを開始
    idling_timer_start();

    // BLEチャネル初期化完了
    data_channel_initialized();
}

static void ble_connected(void)
{
    // BLE接続アイドルタイマーを停止
    app_timer_stop_for_idling();
}

static void ble_disconnected(void)
{
    if (app_ble_pairing_mode() == false) {
        // 非ペアリングモード時は、
        // BLE接続アイドルタイマーを停止-->再開
        idling_timer_start();

        // ペアリング解除要求時は、
        // 接続の切断検知時点でペアリング情報を削除
        fido_ble_unpairing_on_disconnect();
        return;
    }

    if (app_ble_advertise_is_stopped()) {
        // 接続障害時にアドバタイズが停止された場合は
        // 以降の処理を行わない
        return;
    }

    // BLE切断時の処理
    // ペアリングモード初期設定-->BLEアドバタイズ開始-->LED点灯パターン設定
    initialize_pairing_mode();
}

static void ble_connection_failed(void)
{
    // アドバタイズの停止を指示
    app_ble_advertise_stop();

    // ペアリングモード表示用LEDを高速点滅させ、
    // 再度ペアリングが必要であることを通知
    //
    // 黄色LEDを、秒間５回点滅させる
    fido_status_indicator_pairing_fail(true);
}

static void ble_pairing_failed(void)
{
    // アドバタイズの停止を指示
    app_ble_advertise_stop();

    // ペアリングモード表示用LEDを点滅させ、
    // 再度ペアリングが必要であることを通知
    //
    // 黄色LEDを、秒間２回点滅させる
    fido_status_indicator_pairing_fail(false);
}

static void usb_configured(void)
{
    if (app_main_is_data_channel_initialized()) {
        // 既にBLEチャネルが起動している場合は、
        // システムを再始動させる
        app_board_prepare_for_system_reset();
        return;
    }

    // USBが使用可能になったことを
    // LED点滅制御に通知
    app_status_indicator_notify_usb_available(true);

    // 各種業務処理を実行
    app_main_hid_configured();
}

static void usb_disconnected(void)
{
    // システムを再始動させる
    app_board_prepare_for_system_reset();
}

static void ble_idling_detected(void)
{
    // LED点滅管理タイマーを停止し、全LEDを消灯
    app_timer_stop_for_blinking();
    app_status_indicator_light_all(false);

    // ディープスリープ（system off）状態に遷移
    // --> ボタン押下でシステムが再始動
    app_board_prepare_for_deep_sleep();
}

static void enter_to_bootloader(void)
{
    // ブートローダーに制御を移すため、システムを再始動
    app_board_prepare_for_system_reset();
}

static void led_blink_begin(void)
{
    if (app_status_indicator_is_usb_available()) {
        // USBチャネル初期化完了
        data_channel_initialized();
        // アイドル時のLED点滅パターンを設定
        app_status_indicator_idle();
        // 汎用ステータスを削除
        app_flash_general_status_flag_reset();

    } else {
        // USBが使用可能でない場合、汎用ステータスの設定を参照
        bool flag = app_flash_general_status_flag();
        // 次回起動時の判定のため、先に汎用ステータスを設定しておく
        app_flash_general_status_flag_set();
        // 汎用ステータスが設定されていない場合、スリープ状態に遷移
        if (flag == false) {
            app_event_notify(APEVT_IDLING_DETECTED);
            return;
        }

        // ペアリングモード初期設定-->BLEアドバタイズ開始-->LED点灯パターン設定
        initialize_pairing_mode();
    }

    // LED点滅管理用のタイマーを始動
    //   100msごとにAPEVT_LED_BLINKが通知される
    app_timer_start_for_blinking(100, APEVT_LED_BLINK);
}

static void led_blink(void)
{
    // LED点滅管理を実行
    app_status_indicator_blink();
}

void app_process_for_event(APP_EVENT_T event)
{
    // イベントに対応する処理を実行
    switch (event) {
        case APEVT_SUBSYS_INIT:
            app_main_subsys_init();
            break;
        case APEVT_BUTTON_PUSHED:
        case APEVT_BUTTON_RELEASED:
            button_pressed(event);
            break;
        case APEVT_BUTTON_PUSHED_LONG:
            button_pushed_long();
            break;
        case APEVT_BUTTON_1_RELEASED:
            button_1_pressed();
            break;
        case APEVT_BLE_AVAILABLE:
            ble_available();
            break;
        case APEVT_BLE_UNAVAILABLE:
            ble_unavailable();
            break;
        case APEVT_BLE_ADVERTISE_STARTED:
            ble_advertise_started();
            break;
        case APEVT_BLE_CONNECTED:
            ble_connected();
            break;
        case APEVT_BLE_DISCONNECTED:
            ble_disconnected();
            break;
        case APEVT_BLE_CONNECTION_FAILED:
            ble_connection_failed();
            break;
        case APEVT_BLE_PAIRING_FAILED:
            ble_pairing_failed();
            break;
        case APEVT_USB_CONFIGURED:
            usb_configured();
            break;
        case APEVT_USB_DISCONNECTED:
            usb_disconnected();
            break;
        case APEVT_IDLING_DETECTED:
            ble_idling_detected();
            break;
        case APEVT_ENTER_TO_BOOTLOADER:
            enter_to_bootloader();
            break;
        case APEVT_LED_BLINK_BEGIN:
            led_blink_begin();
            break;
        case APEVT_LED_BLINK:
            led_blink();
            break;
        case APEVT_HID_REQUEST_RECEIVED:
            app_main_hid_request_received();
            break;
        case APEVT_BLE_REQUEST_RECEIVED:
            app_main_ble_request_received();
            break;
        case APEVT_CCID_REQUEST_RECEIVED:
            app_main_ccid_request_received();
            break;
        case APEVT_APP_SETTINGS_SAVED:
            app_main_app_settings_saved();
            break;
        case APEVT_APP_SETTINGS_DELETED:
            app_main_app_settings_deleted();
            break;
        case APEVT_APP_CRYPTO_INIT_DONE:
            app_main_app_crypto_init_done();
            break;
        case APEVT_APP_CRYPTO_RANDOM_PREGEN_DONE:
            app_main_app_crypto_done();
            break;
        default:
            break;
    }
}

//
// データ処理イベント
//
void app_process_for_data_event(DATA_EVENT_T event, uint8_t *data, size_t size)
{
    // イベントに対応する処理を実行
    switch (event) {
        case DATEVT_HID_DATA_FRAME_RECEIVED:
            app_main_hid_data_frame_received(data, size);
            break;
        case DATEVT_HID_REPORT_SENT:
            app_main_hid_report_sent();
            break;
        case DATEVT_CCID_DATA_FRAME_RECEIVED:
            app_main_ccid_data_frame_received(data, size);
            break;
        case DATEVT_BLE_DATA_FRAME_RECEIVED:
            app_main_ble_data_frame_received(data, size);
            break;
        case DATEVT_BLE_RESPONSE_SENT:
            app_main_ble_response_sent();
            break;
        default:
            break;
    }
}
