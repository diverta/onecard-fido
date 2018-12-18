/* 
 * File:   fido_common.h
 * Author: makmorit
 *
 * Created on 2018/12/18, 11:09
 */
#ifndef FIDO_COMMON_H
#define FIDO_COMMON_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

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

