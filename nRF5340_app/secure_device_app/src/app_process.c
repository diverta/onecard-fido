/* 
 * File:   app_process.c
 * Author: makmorit
 *
 * Created on 2021/04/28, 10:22
 */
#include <zephyr/types.h>
#include <zephyr.h>

#include "app_ble_pairing.h"
#include "app_bluetooth.h"
#include "app_board.h"
#include "app_event.h"
#include "app_main.h"
#include "app_timer.h"

// ログ出力制御
#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_process);

//
// ボタンイベント振分け処理
//
static void button_pushed_long(void)
{
    // ボタン押下後、３秒経過した時の処理
    if (app_ble_pairing_mode() == false) {
        // 非ペアリングモード時は、
        // ペアリングモード遷移前に、LED1を点灯させる
        app_board_led_light(LED_COLOR_YELLOW, true);
    }
}

static void button_pressed_long(void)
{
    // ボタン押下-->３秒経過後にボタンを離した時の処理
    LOG_DBG("Long pushed");
    if (app_ble_pairing_mode() == false) {
        // 非ペアリングモード時は、
        // ペアリングモード遷移-->アドバタイズ再開
        if (app_ble_pairing_mode_set(true)) {
            app_ble_start_advertising();
        }
    }
}

static void button_pressed_short(void)
{
    // ボタン押下-->３秒以内にボタンを離した時の処理
    // 各種業務処理を実行
    app_main_button_pressed_short();
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
    //   タイムアウト＝３分
    app_timer_start_for_idling(180000, APEVT_IDLING_DETECTED);
    timer_started = true;
}

static void ble_advertise_started(void)
{
    // BLE接続アイドルタイマーを開始
    idling_timer_start();
}

static void ble_connected(void)
{
    // BLE接続アイドルタイマーを停止
    app_timer_stop_for_idling();
}

static void ble_disconnected(void)
{
    // BLE接続アイドルタイマーを開始
    idling_timer_start();

    // BLE切断時の処理
    if (app_ble_pairing_mode()) {
        // ペアリングモード時は、
        // 非ペアリングモード遷移前に、LED1を消灯させる
        app_board_led_light(LED_COLOR_YELLOW, false);
        // 非ペアリングモード遷移-->アドバタイズ再開
        if (app_ble_pairing_mode_set(false)) {
            app_ble_start_advertising();
        }
    }
}

static void usb_configured(void)
{
    // BLEアドバタイズを停止させる
    app_ble_stop_advertising();

    // BLE接続アイドルタイマーを停止
    app_timer_stop_for_idling();
}

static void usb_disconnected(void)
{
    // システムを再始動させる
    app_board_prepare_for_system_reset();
}

static void ble_idling_detected(void)
{
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
    // TODO: 
    // アイドル時のLED点滅パターンを設定

    // LED点滅管理用のタイマーを始動
    //   100msごとにAPEVT_LED_BLINKが通知される
    app_timer_start_for_blinking();
}

static void led_blink(void)
{
    // TODO: 
    // LED点滅管理を実行
}

void app_process_for_event(APP_EVENT_T event)
{
    // イベントに対応する処理を実行
    switch (event) {
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
        case APEVT_BLE_ADVERTISE_STARTED:
            ble_advertise_started();
            break;
        case APEVT_BLE_CONNECTED:
            ble_connected();
            break;
        case APEVT_BLE_DISCONNECTED:
            ble_disconnected();
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
        case DATEVT_HID_REPORT_RECEIVED:
            app_main_hid_report_received(data, size);
            break;
        case DATEVT_HID_REPORT_SENT:
            app_main_hid_report_sent();
            break;
        case DATEVT_CCID_DATA_RECEIVED:
            app_main_ccid_data_received(data, size);
            break;
        default:
            break;
    }
}

//
// 業務処理初期化
//
void app_process_initialize(void)
{
    // LED点滅管理用のタイマーを
    // 500ms後に始動させるようにする
    //   500ms wait --> 
    //   APEVT_LED_BLINK_BEGINが通知される
    app_timer_start_for_blinking_begin(500);
}
