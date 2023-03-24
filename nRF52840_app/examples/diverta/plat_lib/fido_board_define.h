/* 
 * File:   fido_board_define.h
 * Author: makmorit
 *
 * Created on 2022/12/30, 15:24
 */
#ifndef FIDO_BOARD_DEFINE_H
#define FIDO_BOARD_DEFINE_H

#include <sdk_config.h>
#include <nrf_gpio.h>
#include <nrf_delay.h>

#ifdef __cplusplus
extern "C" {
#endif

// ボード情報管理
//   アプリケーションのバージョン
#ifndef FW_REV
#define FW_REV              "0.0.0"
#endif
//   アプリケーションのビルド番号
#ifndef FW_BUILD
#define FW_BUILD            "0"
#endif
//   ハードウェアのバージョン
#ifndef HW_REV
#define HW_REV              ""
#endif
//   ボード名称
#if defined(BOARD_PCA10056)
#define DEVICE_NAME         "Secure DK"
#else
#define DEVICE_NAME         "Secure Dongle"
#endif
//   Device Information Service（BLE）が提供する情報
#define MANUFACTURER_NAME   "Diverta Inc."
#define MODEL_NUM           "0001"

// SCL/SDA signal pin
#if defined(BOARD_PCA10056)
#define TWI_SCL_PIN     NRF_GPIO_PIN_MAP(0,27)
#define TWI_SDA_PIN     NRF_GPIO_PIN_MAP(0,26)
#else
#define TWI_SCL_PIN     NRF_GPIO_PIN_MAP(0,2)
#define TWI_SDA_PIN     NRF_GPIO_PIN_MAP(0,6)
#endif

//
// TFT接続時のピン定義
//
#define SPI_SCK_PIN     NRF_GPIO_PIN_MAP(1,15)
#define SPI_MISO_PIN    NRF_GPIO_PIN_MAP(1,14)
#define SPI_MOSI_PIN    NRF_GPIO_PIN_MAP(1,13)
#define TFT_C_S         NRF_GPIO_PIN_MAP(1,12)
#define TFT_RESET       NRF_GPIO_PIN_MAP(1,11)
#define TFT_D_C         NRF_GPIO_PIN_MAP(1,10)
#define TFT_LED         NRF_GPIO_PIN_MAP(1,9)

//
// LEDのピン
//
#if defined(BOARD_PCA10059)
#define LED_Y   LED1_G                  // rev2 LED3 (yellow)
#define LED_R   NRF_GPIO_PIN_MAP(1,10)  // rev2 LED1 (red)
#elif defined(BOARD_PCA10059_02)
#define LED_Y   LED1_G                  // rev2.1.2 LED1 (yellow)
#define LED_R   LED_2                   // rev2.1.2 LED2 (red)
#else
#define LED_Y   LED_1
#define LED_R   LED_2
#endif
#define LED_G   LED_3
#define LED_B   LED_4

// LED種別
typedef enum _LED_COLOR {
    LED_COLOR_NONE = 0,
    LED_COLOR_RED,
    LED_COLOR_GREEN,
    LED_COLOR_BLUE,
    LED_COLOR_BUSY,
    LED_COLOR_PAIR
} LED_COLOR;

// ピン設定マクロ
#define fido_board_gpio_cfg_output(pin_number) nrf_gpio_cfg_output(pin_number)
#define fido_board_gpio_pin_clear(pin_number)  nrf_gpio_pin_clear(pin_number)
#define fido_board_gpio_pin_set(pin_number)    nrf_gpio_pin_set(pin_number)

// 遅延用マクロ
#define fido_board_delay_us(us_time) nrf_delay_us(us_time)
#define fido_board_delay_ms(ms_time) nrf_delay_ms(ms_time)

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BOARD_DEFINE_H */
