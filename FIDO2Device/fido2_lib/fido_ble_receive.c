/* 
 * File:   fido_ble_receive.c
 * Author: makmorit
 *
 * Created on 2019/06/26, 11:32
 */
//
// プラットフォーム非依存コード
//
#include "fido_ble_receive.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// リクエストデータに含まれるBLEヘッダー、APDU項目は
// このモジュール内で保持
static BLE_HEADER_T ble_header_t;
static FIDO_APDU_T  apdu_t;

BLE_HEADER_T *fido_ble_receive_header(void)
{
    return &ble_header_t;
}

FIDO_APDU_T *fido_ble_receive_apdu(void)
{
    return &apdu_t;
}

//
// 経過措置
//   ble_u2f_commandで判定されたコマンドを保持
//
static enum COMMAND_TYPE command;

enum COMMAND_TYPE fido_ble_receive_command_get(void)
{
    return command;
}

void fido_ble_receive_command_set(enum COMMAND_TYPE c)
{
    command = c;
}
