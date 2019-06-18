/* 
 * File:   fido_board.h
 * Author: makmorit
 *
 * Created on 2019/06/18, 10:12
 */

#ifndef FIDO_BOARD_H
#define FIDO_BOARD_H

#include "boards.h"

#ifdef __cplusplus
extern "C" {
#endif

// FIDO機能で使用するLEDのピン番号を設定
// nRF52840 Dongleでは以下の割り当てになります。
//   LED2=Red
//   LED3=Green
//   LED4=Blue
#define LED_FOR_PAIRING_MODE    LED_2
#define LED_FOR_USER_PRESENCE   LED_3
#define LED_FOR_PROCESSING      LED_4

// 点滅間隔の定義
#define LED_ON_OFF_INTERVAL_MSEC        300
#define LED_ON_OFF_SHORT_INTERVAL_MSEC  100
#define LED_BLINK_INTERVAL_MSEC         250

// 関数群
void fido_led_light_LED(uint32_t pin_number, bool led_on);
void fido_led_light_all_LED(bool led_on);
void fido_processing_led_timedout_handler(void);
void fido_processing_led_on(uint32_t led_for_processing, uint32_t on_off_interval_msec);
void fido_processing_led_off(void);
void fido_idling_led_timedout_handler(void);
void fido_idling_led_on(void);
void fido_idling_led_off(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BOARD_H */

