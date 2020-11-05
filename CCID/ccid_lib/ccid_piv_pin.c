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
// 共通処理
//
static uint16_t verify_pin_code(command_apdu_t *capdu) 
{
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

//
// PIN設定処理
//
uint16_t ccid_piv_pin_set(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // パラメーターのチェック
    if (capdu->p1 != 0x00) {
        return SW_WRONG_P1P2;
    }
    PIV_PIN_TYPE pin_type;
    if (capdu->p2 == 0x80) {
        pin_type = PIV_PIN;
    } else if (capdu->p2 == 0x81) {
        pin_type = PIV_PUK;
    } else {
        return SW_REFERENCE_DATA_NOT_FOUND;
    }
    if (capdu->lc != 16) {
        return SW_WRONG_LENGTH;
    }

    // 入力されたPIN or PUKで認証
    uint16_t ret = verify_pin_code(capdu);
    if (ret != SW_NO_ERROR) {
        return ret;
    }

    // 認証済みフラグをリセット
    pin_is_validated = false;

    // PIN or PUKを更新
    uint8_t *update_pin = capdu->data + PIN_DEFAULT_SIZE;
    if (ccid_pin_update(pin_type, update_pin, PIN_DEFAULT_SIZE) == false) {
        // PINが更新できない場合は処理失敗
        return SW_UNABLE_TO_PROCESS;
    }

    // 正常終了
    return SW_NO_ERROR;
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
    return verify_pin_code(capdu);
}
