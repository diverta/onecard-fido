/* 
 * File:   fido_common.h
 * Author: makmorit
 *
 * Created on 2018/12/18, 11:09
 */
#ifndef FIDO_COMMON_H
#define FIDO_COMMON_H

#include <stdbool.h>
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

// FIDO機能関連エラーステータス
#define CTAP1_ERR_INVALID_COMMAND   0x01
#define CTAP1_ERR_INVALID_PARAMETER 0x02
#define CTAP1_ERR_INVALID_LENGTH    0x03
#define CTAP1_ERR_INVALID_SEQ       0x04
#define CTAP1_ERR_TIMEOUT           0x05
#define CTAP1_ERR_CHANNEL_BUSY      0x06
#define CTAP1_ERR_LOCK_REQUIRED     0x0a
#define CTAP1_ERR_OTHER             0x7f

// リクエストデータに含まれるAPDU項目を保持
typedef struct {
    uint8_t  CLA;
    uint8_t  INS;
    uint8_t  P1;
    uint8_t  P2;
    uint32_t Lc;
    uint8_t *data;
    uint32_t data_length;
    uint32_t Le;
} FIDO_APDU_T;

// APDUに格納できるデータ長の上限
#ifndef APDU_DATA_MAX_LENGTH
#define APDU_DATA_MAX_LENGTH 1024
#endif

// 関数群
void fido_led_light_LED(uint32_t pin_number, bool led_on);
void fido_set_status_word(uint8_t *dest_buffer, uint16_t status_word);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_COMMON_H */

