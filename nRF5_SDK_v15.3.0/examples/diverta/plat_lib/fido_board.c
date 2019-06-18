/* 
 * File:   fido_common.c
 * Author: makmorit
 *
 * Created on 2018/12/18, 11:09
 */
#include "sdk_common.h"

// for lighting LED
#include "nrf_gpio.h"

#include "fido_board.h"

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

void fido_led_light_all_LED(bool led_on)
{
    fido_led_light_LED(LED_FOR_PAIRING_MODE, led_on);
    fido_led_light_LED(LED_FOR_USER_PRESENCE, led_on);
    fido_led_light_LED(LED_FOR_PROCESSING, led_on);
}
