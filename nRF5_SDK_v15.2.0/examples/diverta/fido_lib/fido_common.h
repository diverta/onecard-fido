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
#define CTAP1_ERR_SUCCESS               0x00
#define CTAP1_ERR_INVALID_COMMAND       0x01
#define CTAP1_ERR_INVALID_PARAMETER     0x02
#define CTAP1_ERR_INVALID_LENGTH        0x03
#define CTAP1_ERR_INVALID_SEQ           0x04
#define CTAP1_ERR_TIMEOUT               0x05
#define CTAP1_ERR_CHANNEL_BUSY          0x06
#define CTAP1_ERR_LOCK_REQUIRED         0x0a
#define CTAP1_ERR_INVALID_CHANNEL       0x0b
#define CTAP2_ERR_CBOR_PARSING          0x10
#define CTAP2_ERR_CBOR_UNEXPECTED_TYPE  0x11
#define CTAP2_ERR_INVALID_CBOR          0x12
#define CTAP2_ERR_INVALID_CBOR_TYPE     0x13
#define CTAP2_ERR_MISSING_PARAMETER     0x14
#define CTAP2_ERR_LIMIT_EXCEEDED        0x15
#define CTAP2_ERR_TOO_MANY_ELEMENTS     0x17
#define CTAP2_ERR_CREDENTIAL_EXCLUDED   0x19
#define CTAP2_ERR_PROCESSING            0x21
#define CTAP2_ERR_UNSUPPORTED_ALGORITHM 0x26
#define CTAP2_ERR_INVALID_OPTION        0x2c
#define CTAP2_ERR_KEEPALIVE_CANCEL      0x2d
#define CTAP2_ERR_NO_CREDENTIALS        0x2e
#define CTAP2_ERR_PIN_INVALID           0x31
#define CTAP2_ERR_PIN_BLOCKED           0x32
#define CTAP2_ERR_PIN_AUTH_INVALID      0x33
#define CTAP2_ERR_PIN_AUTH_BLOCKED      0x34
#define CTAP2_ERR_PIN_NOT_SET           0x35
#define CTAP2_ERR_PIN_POLICY_VIOLATION  0x37
#define CTAP1_ERR_OTHER                 0x7f
#define CTAP2_ERR_SPEC_LAST             0xdf
#define CTAP2_ERR_EXTENSION_FIRST       0xe0
#define CTAP2_ERR_EXTENSION_LAST        0xef
#define CTAP2_ERR_VENDOR_FIRST          0xf0
#define CTAP2_ERR_VENDOR_LAST           0xff

// 管理コマンドの識別用
#define MNT_COMMAND_BASE                0xc0
#define MNT_COMMAND_ERASE_SKEY_CERT     0xc0
#define MNT_COMMAND_INSTALL_SKEY_CERT   0xc1

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
void fido_led_light_all_LED(bool led_on);
void fido_set_status_word(uint8_t *dest_buffer, uint16_t status_word);
void fido_set_uint32_bytes(uint8_t *p_dest_buffer, uint32_t bytes);
void fido_set_uint16_bytes(uint8_t *p_dest_buffer, uint16_t bytes);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_COMMON_H */
