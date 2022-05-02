/* 
 * File:   ccid_piv_pin_auth.c
 * Author: makmorit
 *
 * Created on 2020/11/10, 14:48
 */
#include <string.h>

#include "ccid_apdu.h"
#include "ccid_piv_pin_auth.h"
#include "ccid_piv_object.h"

//
// PIN認証処理関連
//
static bool    m_pin_auth_failed;
static uint8_t m_pin_current_retries;

static bool    m_puk_auth_failed;
static uint8_t m_puk_current_retries;

bool ccid_piv_pin_auth_failed(uint8_t pin_type)
{
    // 認証NGの場合は true
    if (pin_type == TAG_KEY_PUK) {
        return m_puk_auth_failed;
    } else {
        return m_pin_auth_failed;
    }
}

uint8_t ccid_piv_pin_auth_current_retries(uint8_t pin_type)
{
    // 現在のリトライカウンターを戻す
    if (pin_type == TAG_KEY_PUK) {
        return m_puk_current_retries;
    } else {
        return m_pin_current_retries;
    }
}

static void auth_failed_set(uint8_t pin_type, bool b)
{
    // 認証OK／NGの別を設定
    if (pin_type == TAG_KEY_PUK) {
        m_puk_auth_failed = b;
    } else {
        m_pin_auth_failed = b;
    }
}

static void current_retries_set(uint8_t pin_type, uint8_t retries)
{
    // 現在のリトライカウンターを設定
    if (pin_type == TAG_KEY_PUK) {
        m_puk_current_retries = retries;
    } else {
        m_pin_current_retries = retries;
    }
}

//
// 関数群
//
static bool restore_pin_retries(uint8_t pin_type, uint8_t *curr)
{
    // パラメーターチェック
    if (curr == NULL) {
        return false;
    }
    // 登録されているリトライカウンターを取得
    if (ccid_piv_object_pin_get(pin_type, NULL, curr) == false) {
        return false;
    }
    return true;
}

static bool restore_pin_code(uint8_t pin_type, uint8_t *pin_buf)
{
    // 登録されているPINを取得
    if (ccid_piv_object_pin_get(pin_type, pin_buf, NULL) == false) {
        return false;
    }
    return true;
}

// 一時読込み用領域
static uint8_t pin_buf[PIN_DEFAULT_SIZE];

//
// 業務処理群
//
static bool pin_code_is_equal(uint8_t pin_type, const void *buf, uint8_t len, bool *is_equal)
{
    // パラメーターチェック
    if (is_equal == NULL) {
        return false;
    }

    // 登録されているPINを取得
    if (restore_pin_code(pin_type, pin_buf) == false) {
        return false;
    }

    // 入力されたPINをチェック
    *is_equal = (memcmp(buf, pin_buf, len) == 0);
    memset(pin_buf, 0, sizeof(pin_buf));
    return true;
}

static bool pin_code_is_blank(uint8_t pin_type, bool *is_blank)
{
    // パラメーターチェック
    if (is_blank == NULL) {
        return false;
    }

    // 登録されているPINを取得
    if (restore_pin_code(pin_type, pin_buf) == false) {
        return false;
    }

    // 登録されているPINをチェック
    *is_blank = true;
    for (uint8_t c = 0; c < sizeof(pin_buf); c++) {
        if (pin_buf[c] != 0xff) {
            *is_blank = false;
            break;
        }
    }

    // PINがブランクであれば true を戻す
    memset(pin_buf, 0, sizeof(pin_buf));
    return true;
}

uint16_t ccid_piv_pin_auth_verify(uint8_t pin_type, uint8_t *buf, uint8_t len) 
{
    // 内部変数を初期化
    auth_failed_set(pin_type, true);
    current_retries_set(pin_type, 0);

    // パラメーターチェック
    if (buf == NULL) {
        return SW_UNABLE_TO_PROCESS;
    }
    if (len != PIN_DEFAULT_SIZE) {
        return SW_WRONG_LENGTH;
    }

    // リトライカウンターのデフォルト／現在値を参照
    uint8_t current_cnt;
    if (restore_pin_retries(pin_type, &current_cnt) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // リトライカウンターが０であれば認証をブロック
    if (current_cnt == 0) {
        return SW_AUTHENTICATION_BLOCKED;
    }

    // 入力されたPINと、登録されているPINを比較
    bool is_equal;
    if (pin_code_is_equal(pin_type, buf, len, &is_equal) == false) {
        return SW_UNABLE_TO_PROCESS;
    }
    if (is_equal == false) {
        // NGの場合はリトライカウンターを１減らす
        current_retries_set(pin_type, --current_cnt);
    } else {
        // OKの場合はリトライカウンターをデフォルトに設定
        current_retries_set(pin_type, PIN_DEFAULT_RETRY_CNT);
        auth_failed_set(pin_type, false);
    }
    return SW_NO_ERROR;
}

bool ccid_piv_pin_auth_get_retries(uint8_t pin_type, uint8_t *retries) 
{
    // パラメーターチェック
    if (retries == NULL) {
        return false;
    }

    // PINコードがブランクであればゼロクリア
    bool is_blank;
    if (pin_code_is_blank(pin_type, &is_blank) == false) {
        return false;
    }
    if (is_blank) {
        *retries = 0;
        return true;
    }

    // リトライカウンターのデフォルト／現在値を参照
    uint8_t current_cnt;
    if (restore_pin_retries(pin_type, &current_cnt) == false) {
        return false;
    }

    // リトライカウンターの現在地を設定して戻す
    *retries = current_cnt;
    return true;
}
