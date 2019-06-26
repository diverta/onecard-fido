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

//
// 経過措置
//   ble_u2f_commandで判定されたコマンドを保持
//
#include "ble_u2f_command.h"
enum COMMAND_TYPE fido_ble_receive_command_get(void);
void fido_ble_receive_command_set(enum COMMAND_TYPE c);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BLE_RECEIVE_H */
