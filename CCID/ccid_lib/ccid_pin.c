/* 
 * File:   ccid_pin.c
 * Author: makmorit
 *
 * Created on 2020/07/22, 10:47
 */
#include <string.h>

#include "ccid_apdu.h"
#include "ccid_pin.h"
#include "ccid_piv_object.h"

//
// 関数群
//
static bool restore_pin_retries(uint8_t *curr)
{
    // パラメーターチェック
    if (curr == NULL) {
        return false;
    }
    // 登録されているリトライカウンターを取得
    if (ccid_piv_object_pin_get(TAG_PIV_PIN, NULL, curr) == false) {
        return false;
    }
    return true;
}

static bool restore_pin_code(uint8_t *pin_buf)
{
    // 登録されているPINを取得
    if (ccid_piv_object_pin_get(TAG_PIV_PIN, pin_buf, NULL) == false) {
        return false;
    }
    return true;
}

// 一時読込み用領域
static uint8_t pin_buf[PIN_DEFAULT_SIZE];

//
// 業務処理群
//
static bool pin_code_is_equal(const void *buf, uint8_t len, bool *is_equal)
{
    // パラメーターチェック
    if (is_equal == NULL) {
        return false;
    }

    // 登録されているPINを取得
    if (restore_pin_code(pin_buf) == false) {
        return false;
    }

    // 入力されたPINをチェック
    *is_equal = (memcmp(buf, pin_buf, len) == 0);
    memset(pin_buf, 0, sizeof(pin_buf));
    return true;
}

static bool pin_code_is_blank(bool *is_blank)
{
    // パラメーターチェック
    if (is_blank == NULL) {
        return false;
    }

    // 登録されているPINを取得
    if (restore_pin_code(pin_buf) == false) {
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

bool ccid_pin_verify(const void *buf, uint8_t len, uint8_t *retries, bool *auth_failed) 
{
    // パラメーターチェック
    if (retries == NULL) {
        return false;
    }
    if (len != PIN_DEFAULT_SIZE) {
        return false;
    }

    // リトライカウンターのデフォルト／現在値を参照
    uint8_t current_cnt;
    if (restore_pin_retries(&current_cnt) == false) {
        return false;
    }
    *retries = current_cnt;

    // リトライカウンターが０であれば認証失敗
    if (current_cnt == 0) {
        *auth_failed = true;
        return true;
    }

    // 入力されたPINと、登録されているPINを比較
    bool is_equal;
    if (pin_code_is_equal(buf, len, &is_equal) == false) {
        return false;
    }
    if (is_equal == false) {
        // NGの場合はリトライカウンターを１減らす
        *retries = --current_cnt;
        *auth_failed = true;
    } else {
        // OKの場合はリトライカウンターをデフォルトに設定
        *retries = PIN_DEFAULT_RETRY_CNT;
        *auth_failed = false;
    }
    return true;
}

bool ccid_pin_get_retries(uint8_t *retries) 
{
    // パラメーターチェック
    if (retries == NULL) {
        return false;
    }

    // PINコードがブランクであればゼロクリア
    bool is_blank;
    if (pin_code_is_blank(&is_blank) == false) {
        return false;
    }
    if (is_blank) {
        *retries = 0;
        return true;
    }

    // リトライカウンターのデフォルト／現在値を参照
    uint8_t current_cnt;
    if (restore_pin_retries(&current_cnt) == false) {
        return false;
    }

    // リトライカウンターの現在地を設定して戻す
    *retries = current_cnt;
    return true;
}
