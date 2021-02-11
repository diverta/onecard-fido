/* 
 * File:   ccid_pin_auth.c
 * Author: makmorit
 *
 * Created on 2021/02/11, 9:43
 */
#include <string.h>

#include "ccid_apdu.h"
#include "ccid_pin.h"

//
// PIN種別情報
//
static PIN_T pw1 = {.type = OPGP_PIN_PW1, .size_min = 6, .size_max = 64, .is_validated = false, .default_retries = 3};
static PIN_T pw3 = {.type = OPGP_PIN_PW3, .size_min = 8, .size_max = 64, .is_validated = false, .default_retries = 3};
static PIN_T rc  = {.type = OPGP_PIN_RC,  .size_min = 8, .size_max = 64, .is_validated = false, .default_retries = 3};

PIN_T *ccid_pin_auth_pin_t(PIN_TYPE type)
{
    switch (type) {
        case OPGP_PIN_PW1:
            return &pw1;
        case OPGP_PIN_PW3:
            return &pw3;
        case OPGP_PIN_RC:
            return &rc;
        default:
            return NULL;
    }
}

//
// PIN認証処理関連
//
static bool    m_pin_auth_failed;
static uint8_t m_pin_current_retries;

bool ccid_pin_auth_failed(void)
{
    // 認証NGの場合は true
    return m_pin_auth_failed;
}

uint8_t ccid_pin_auth_current_retries(void)
{
    // 現在のリトライカウンターを戻す
    return m_pin_current_retries;
}

static void auth_failed_set(bool b)
{
    // 認証OK／NGの別を設定
    m_pin_auth_failed = b;
}

static void current_retries_set(uint8_t retries)
{
    // 現在のリトライカウンターを設定
    m_pin_current_retries = retries;
}

// 一時読込み用領域
static uint8_t stored_pin[256];
static size_t  stored_pin_size;
static uint8_t stored_retries;

static uint16_t ccid_pin_func_terminate(uint16_t sw)
{
    // 一時読み込み用領域をクリア
    memset(stored_pin, 0, sizeof(stored_pin));
    stored_pin_size = 0;
    return sw;
}

//
// 関数群
//
static bool restore_pin_object(PIN_TYPE type)
{
    // 登録されているリトライカウンターを取得
    // if (ccid_piv_object_pin_get(type, pin->pin, &pin->retries) == false) {
    //     return false;
    // }

    // TODO: 仮の実装です。
    memset(stored_pin, 0, sizeof(stored_pin));
    stored_pin_size = 0;
    stored_retries = 3;
    switch (type) {
        case OPGP_PIN_PW3:
        case OPGP_PIN_RC:
            memcpy(stored_pin, "12345678", 8);
            stored_pin_size = 8;
            break;
        case OPGP_PIN_PW1:
            memcpy(stored_pin, "123456", 6);
            stored_pin_size = 6;
            break;
        default:
            break;
    }

    return true;
}

static bool pin_code_is_equal(uint8_t *buf, uint8_t len, bool *is_equal)
{
    // パラメーターチェック
    if (buf == NULL || len < 1 || is_equal == NULL) {
        return false;
    }

    // 入力されたPINをチェック
    *is_equal = false;
    if (len == stored_pin_size) {
        if (memcmp(buf, stored_pin, stored_pin_size) == 0) {
            *is_equal = true;
        }
    }
    return true;
}

static bool pin_code_is_blank(bool *is_blank)
{
    // パラメーターチェック
    if (is_blank == NULL) {
        return false;
    }

    // 登録されているPINをチェック
    *is_blank = true;
    for (uint8_t c = 0; c < stored_pin_size; c++) {
        if (stored_pin[c] != 0xff) {
            *is_blank = false;
            break;
        }
    }

    // PINがブランクであれば true を戻す
    return true;
}

uint16_t ccid_pin_auth_verify(PIN_T *pin, uint8_t *buf, uint8_t len) 
{
    // 内部変数を初期化
    auth_failed_set(true);
    current_retries_set(0);

    // パラメーターチェック
    if (pin == NULL || buf == NULL) {
        return SW_UNABLE_TO_PROCESS;
    }
    if (len < pin->size_min || len > pin->size_max) {
        return SW_WRONG_LENGTH;
    }

    // Flash ROMのPINオブジェクトを参照
    if (restore_pin_object(pin->type) == false) {
        return SW_UNABLE_TO_PROCESS;
    }
    // リトライカウンターの現在値を参照
    uint8_t current_cnt = stored_retries;

    // リトライカウンターが０であれば認証をブロック
    if (current_cnt == 0) {
        return ccid_pin_func_terminate(SW_AUTHENTICATION_BLOCKED);
    }

    // 入力されたPINと、登録されているPINを比較
    bool is_equal = false;
    if (pin_code_is_equal(buf, len, &is_equal) == false) {
        return ccid_pin_func_terminate(SW_UNABLE_TO_PROCESS);
    }
    if (is_equal == false) {
        // NGの場合はリトライカウンターを１減らす
        current_retries_set(--current_cnt);
    } else {
        // OKの場合はリトライカウンターをデフォルトに設定
        current_retries_set(pin->default_retries);
        auth_failed_set(false);
    }
    return ccid_pin_func_terminate(SW_NO_ERROR);
}

uint16_t ccid_pin_auth_get_retries(PIN_TYPE type, uint8_t *retries) 
{
    // パラメーターチェック
    if (retries == NULL) {
        return SW_UNABLE_TO_PROCESS;
    }

    // Flash ROMのPINオブジェクトを参照
    if (restore_pin_object(type) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // PINコードがブランクであればゼロクリア
    bool is_blank;
    if (pin_code_is_blank(&is_blank) == false) {
        return ccid_pin_func_terminate(SW_UNABLE_TO_PROCESS);
    }
    if (is_blank) {
        *retries = 0;
        return ccid_pin_func_terminate(SW_NO_ERROR);
    }

    // リトライカウンターの現在値を設定して戻す
    *retries = stored_retries;
    return ccid_pin_func_terminate(SW_NO_ERROR);
}
