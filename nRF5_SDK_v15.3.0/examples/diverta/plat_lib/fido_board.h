/* 
 * File:   fido_board.h
 * Author: makmorit
 *
 * Created on 2019/06/18, 10:12
 */
#ifndef FIDO_BOARD_H
#define FIDO_BOARD_H

#include <stdbool.h>
#include <stdint.h>
#include <sdk_config.h>

#ifdef __cplusplus
extern "C" {
#endif

// ボード情報管理
//   アプリケーションのバージョン
#ifndef FW_REV
#define FW_REV              "0.0.0"
#endif
//   ハードウェアのバージョン
#ifndef HW_REV
#define HW_REV              ""
#endif
//   ボード名称
#if   defined(BOARD_PCA10056)
#define DEVICE_NAME         "FIDO authenticator board"
#elif defined(BOARD_PCA10059)
#define DEVICE_NAME         "FIDO authenticator dongle"
#elif defined(BOARD_PCA10059_02)
#define DEVICE_NAME         "Secure Dongle"
#else
#define DEVICE_NAME         "FIDO authenticator"
#endif
//   Device Information Service（BLE）が提供する情報
#define MANUFACTURER_NAME   "Diverta Inc."
#define MODEL_NUM           "0001"

// SCL/SDA signal pin
#define TWI_SCL_PIN     NRF_GPIO_PIN_MAP(0,2)   // rev2 AIN0
#define TWI_SDA_PIN     NRF_GPIO_PIN_MAP(0,6)   // rev2 UART_TX

//
// LEDのピン
//
#if defined(BOARD_PCA10059)
#define LED_R   NRF_GPIO_PIN_MAP(1,10)  // rev2 LED1
#define LED_Y   LED1_G                  // rev2 LED3
#elif defined(BOARD_PCA10059_02)
#define LED_R   NRF_GPIO_PIN_MAP(0,27)  // rev2.1.1 LED1
#define LED_Y   LED1_G                  // rev2.1.1 LED3
#else
#define LED_R   LED_1
#define LED_Y   LED_2
#endif

// LED種別
typedef enum _LED_COLOR {
    LED_COLOR_NONE = 0,
    LED_COLOR_RED,
    LED_COLOR_GREEN,
    LED_COLOR_BLUE,
    LED_COLOR_BUSY,
    LED_COLOR_PAIR
} LED_COLOR;

// 関数群
void fido_button_timers_init(void);
void fido_button_init(void);
void fido_command_long_push_timer_handler(void *p_context);
void led_light_pin_set(LED_COLOR led_color, bool led_on);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BOARD_H */
