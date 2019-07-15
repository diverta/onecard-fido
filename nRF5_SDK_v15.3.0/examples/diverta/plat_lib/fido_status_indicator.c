/* 
 * File:   fido_status_indicator.c
 * Author: makmorit
 *
 * Created on 2019/07/15, 12:27
 */
#include "fido_board.h"

void fido_status_indicator_none(void)
{
    // すべてのLEDを消灯
    fido_led_light_all(false);
}

void fido_status_indicator_idle(void)
{
    // アイドル時点滅処理を開始
    //  該当のLEDを、２秒ごとに点滅させる
    fido_idling_led_blink_start();
}

void fido_status_indicator_no_idle(void)
{
    // LEDを消灯
    //  アイドル時点滅処理が行われていた場合は
    //  停止する
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
    // TODO:
    // ペアリングモードの場合は、
    // RED LEDの連続点灯とします。
}

void fido_status_indicator_pairing_fail(void)
{
    // ペアリングモードLED点滅を開始し、
    // 再度ペアリングが必要であることを通知
    fido_caution_led_blink_start(LED_ON_OFF_INTERVAL_MSEC);
}

void fido_status_indicator_abort(void)
{
    // 全色LEDを点灯
    fido_led_light_all(true);    
}
