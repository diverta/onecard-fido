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

// ログ出力制御
#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_process);

void app_process_button_pushed_long(void)
{
    // ボタン押下後、３秒経過した時の処理
    if (app_ble_pairing_mode() == false) {
        // 非ペアリングモード時は、
        // ペアリングモード遷移前に、LED1を点灯させる
        app_board_led_light(LED_COLOR_YELLOW, true);
    }
}

void app_process_button_pressed_long(void)
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

void app_process_button_pressed_short(void)
{
    // ボタン押下-->３秒以内にボタンを離した時の処理
    LOG_DBG("Short pushed");
}

void app_process_ble_connected(void)
{
}

void app_process_ble_disconnected(void)
{
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
