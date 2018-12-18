/* 
 * File:   fido_common.c
 * Author: makmorit
 *
 * Created on 2018/12/18, 11:09
 */
#include "sdk_common.h"
#include <stdbool.h>

// for lighting LED
#include "nrf_gpio.h"

void fido_led_light_LED(uint32_t pin_number, bool led_on)
{
    // LEDを出力設定
    nrf_gpio_cfg_output(pin_number);
    if (led_on == false) {
        // LEDを点灯させる
        nrf_gpio_pin_set(pin_number);
    } else {
        // LEDを消灯させる
        nrf_gpio_pin_clear(pin_number);
    }
}

void fido_set_status_word(uint8_t *dest_buffer, uint16_t status_word)
{
    // ステータスワードをビッグエンディアンで格納
    dest_buffer[0] = (status_word >> 8) & 0x00ff;
    dest_buffer[1] = (status_word >> 0) & 0x00ff;
}
