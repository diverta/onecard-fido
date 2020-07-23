/* 
 * File:   ccid_piv_pin.c
 * Author: makmorit
 *
 * Created on 2020/07/22, 13:05
 */
#include <string.h>

#include "ccid_pin.h"
#include "ccid_piv_pin.h"

// PIN関連情報を内部保持
static bool    pin_is_validated;

// PINポリシー
static uint8_t pin_policy[] = {0x40, 0x10};

bool ccid_piv_pin_is_validated(void)
{
    return pin_is_validated;
}

void ccid_piv_pin_set_validated(bool b)
{
    pin_is_validated = b;
}

uint8_t *ccid_piv_pin_policy(void)
{
    return pin_policy;
}

size_t ccid_piv_pin_policy_size(void)
{
    return sizeof(pin_policy);
}

bool ccid_piv_pin_init(void)
{
    // 内部保持データの初期化
    pin_is_validated = false;

    // PIN／リトライカウンターの初期化
    if (ccid_pin_create(PIN_DEFAULT_CODE, PIN_DEFAULT_SIZE, PIN_DEFAULT_RETRY_CNT) == false) {
        return false;
    }

    return true;
}

//
// PIN認証処理
//
uint16_t ccid_piv_pin_auth(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // パラメーターのチェック
    if (capdu->p1 != 0x00 && capdu->p1 != 0xff) {
        return SW_WRONG_P1P2;
    }
    if (capdu->p2 != 0x80) {
        return SW_REFERENCE_DATA_NOT_FOUND;
    }
    if (capdu->p1 == 0xff) {
        if (capdu->lc != 0) {
            return SW_WRONG_LENGTH;
        }
        pin_is_validated = false;
        return SW_NO_ERROR;
    }

    // リトライカウンター照会の場合
    if (capdu->lc == 0) {
        if (pin_is_validated) {
            return SW_NO_ERROR;
        }
        uint8_t retries;
        if (ccid_pin_get_retries(&retries) == false) {
            return SW_UNABLE_TO_PROCESS;
        }
        // 現在のリトライカウンターを戻す
        return SW_PIN_RETRIES + retries;
    }

    // 入力されたPINが８文字でない場合はエラー
    if (capdu->lc != PIN_DEFAULT_SIZE) {
        return SW_WRONG_LENGTH;
    }

    // 入力されたPINで認証実行
    uint8_t *cdata = capdu->data;
    uint8_t  count;
    bool     pin_auth_failed;
    pin_is_validated = false;
    if (ccid_pin_verify(cdata, PIN_DEFAULT_SIZE, &count, &pin_auth_failed) == false) {
        // 登録されたPINが照会できない場合は処理失敗
        return SW_UNABLE_TO_PROCESS;
    }
    if (count == 0) {
        // リトライカウンターが0であれば認証をブロック
        return SW_AUTHENTICATION_BLOCKED;
    }
    if (pin_auth_failed) {
        // 認証NGの場合は、現在のリトライカウンターを戻す
        return SW_PIN_RETRIES + count;
    }

    // 正常終了
    pin_is_validated = true;
    return SW_NO_ERROR;
}
