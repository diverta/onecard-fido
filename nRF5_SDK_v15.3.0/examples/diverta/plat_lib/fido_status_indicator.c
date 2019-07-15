/* 
 * File:   fido_status_indicator.c
 * Author: makmorit
 *
 * Created on 2019/07/15, 12:27
 */
#include "fido_board.h"
#include "fido_ble_peripheral.h"

void fido_status_indicator_none(void)
{
    // すべてのLEDを消灯
    fido_led_light_all(false);
}

void fido_status_indicator_idle(void)
{
    if (fido_ble_peripheral_mode()) {
        // BLEペリフェラル稼働中かつ
        // 非ペアリングモード＝BLUE LED点滅
        fido_idling_led_ble_blink_start();

    } else {
        // USB HID稼働中＝GREEN LED点滅
        fido_idling_led_blink_start();
    }
}

void fido_status_indicator_no_idle(void)
{
    // LEDを消灯
    // 点滅処理が行われていた場合は停止する
    fido_led_blink_stop();
    fido_idling_led_blink_stop();
}

void fido_status_indicator_prompt_reset(void)
{
    // 赤色LED高速点滅開始
    fido_caution_led_blink_start(LED_ON_OFF_SHORT_INTERVAL_MSEC);
}

void fido_status_indicator_prompt_tup(void)
{
    // 該当のLEDを、秒間２回点滅させる
    fido_prompt_led_blink_start(LED_ON_OFF_INTERVAL_MSEC);
}

void fido_status_indicator_pairing_mode(void)
{
    // ペアリングモードの場合は、
    // RED LEDの連続点灯とします。
    fido_idling_led_ble_pairing_mode();
}

void fido_status_indicator_pairing_fail(void)
{
    // 点滅処理が行われていた場合は停止する
    fido_idling_led_blink_stop();

    // ペアリングモードLED点滅を開始し、
    // 再度ペアリングが必要であることを通知
    fido_caution_led_blink_start(LED_ON_OFF_INTERVAL_MSEC);
}

void fido_status_indicator_abort(void)
{
    // 全色LEDを点灯
    fido_led_light_all(true);    
}
