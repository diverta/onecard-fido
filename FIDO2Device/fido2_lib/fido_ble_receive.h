/* 
 * File:   fido_ble_receive.h
 * Author: makmorit
 *
 * Created on 2019/06/26, 11:32
 */
#ifndef FIDO_BLE_RECEIVE_H
#define FIDO_BLE_RECEIVE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "fido_common.h"
    
// リクエストデータに含まれるBLEヘッダーを保持
typedef struct {
    uint8_t  CMD;
    uint32_t LEN;
    uint8_t  SEQ;

    // リクエストデータの検査中に
    // 確認されたエラーを保持
    uint8_t ERROR;

    // リクエストデータの検査中に
    // 設定されたステータスワードを保持
    uint16_t STATUS_WORD;

    // 後続リクエストがあるかどうかを保持
    bool CONT;
} BLE_HEADER_T;

BLE_HEADER_T *fido_ble_receive_header(void);
FIDO_APDU_T  *fido_ble_receive_apdu(void);

void          fido_ble_receive_init(void);
void          fido_ble_receive_control_point(uint8_t *data, uint16_t length);
void          fido_ble_receive_frame_count_clear(void);
uint8_t       fido_ble_receive_frame_count(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BLE_RECEIVE_H */
