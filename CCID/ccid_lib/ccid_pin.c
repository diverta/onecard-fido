/* 
 * File:   ccid_pin.c
 * Author: makmorit
 *
 * Created on 2020/07/22, 10:47
 */
#include <string.h>

#include "ccid_pin.h"

//
// 仮の実装です。
//
// リトライカウンター／PINコードの永続化領域
//
static uint16_t m_default_retries;
static uint16_t m_current_retries;
static uint8_t  m_pin_buffer[PIN_DEFAULT_SIZE];
//
// 関数群
//
static bool save_pin_retries_default(uint8_t retries)
{
    m_default_retries = retries;
    return true;
}
static bool save_pin_retries_current(uint8_t retries)
{
    m_current_retries = retries;
    return true;
}
static bool restore_pin_retries(uint8_t *def, uint8_t *curr)
{
    // パラメーターチェック
    if (def == NULL || curr == NULL) {
        return false;
    }
    // 登録されているリトライカウンターを取得
    *def = m_default_retries;
    *curr = m_current_retries;
    return true;
}
static bool save_pin_code(uint8_t *pin_buf, size_t pin_buf_size)
{
    // 登録されているPINを取得
    memset(m_pin_buffer, 0xff, sizeof(m_pin_buffer));
    memcpy(m_pin_buffer, pin_buf, pin_buf_size);
    return true;
}
static bool restore_pin_code(uint8_t *pin_buf, size_t pin_buf_size)
{
    // 登録されているPINを取得
    memset(pin_buf, 0xff, pin_buf_size);
    memcpy(pin_buf, m_pin_buffer, pin_buf_size);
    return true;
}
//
// 仮の実装です（ここまで）
//

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
    if (restore_pin_code(pin_buf, sizeof(pin_buf)) == false) {
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
    if (restore_pin_code(pin_buf, sizeof(pin_buf)) == false) {
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
    uint8_t default_cnt;
    uint8_t current_cnt;
    if (restore_pin_retries(&default_cnt, &current_cnt) == false) {
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
        // 現在のリトライカウンターを更新
        if (save_pin_retries_current(current_cnt) == false) {
            return false;
        }
        *auth_failed = true;
        return true;
    }

    // 認証が成功したら、リトライカウンターをデフォルト値に再設定
    if (save_pin_retries_current(default_cnt) == false) {
        return false;
    }
    *auth_failed = false;
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
    uint8_t default_cnt;
    uint8_t current_cnt;
    if (restore_pin_retries(&default_cnt, &current_cnt) == false) {
        return false;
    }

    // リトライカウンターの現在地を設定して戻す
    *retries = current_cnt;
    return true;
}

//
// PIN更新処理
//
static bool update_pin(const void *buf, uint8_t len)
{
    uint8_t default_cnt;
    uint8_t current_cnt;

    // PINを更新
    if (save_pin_code((uint8_t *)buf, len) == false) {
        return false;
    }
    // リトライカウンターのデフォルトを参照
    if (restore_pin_retries(&default_cnt, &current_cnt) == false) {
        return false;
    }
    // PINのリトライカウンターをデフォルトに設定
    if (save_pin_retries_current(default_cnt) == false) {
        return false;
    }
    // 処理成功
    return true;
}

bool ccid_pin_update(PIV_PIN_TYPE type, const void *buf, uint8_t len) 
{
    bool ret = false;
    switch (type) {
        case PIV_PIN:
            // PINを更新
            ret = update_pin(buf, len);
            break;
        default:
            // Not supported
            break;
    }
    return ret;
}

//
// 仮の実装です。
//
bool ccid_pin_create(const void *buf, uint8_t len, uint8_t max_retries) 
{
    // PINを設定
    if (save_pin_code((uint8_t *)buf, len) == false) {
        return false;
    }

    // 内部保持用のリトライカウンターを設定
    if (save_pin_retries_current(max_retries) == false) {
        return false;
    }

    // デフォルトのリトライカウンターを設定
    if (save_pin_retries_default(max_retries) == false) {
        return false;
    }

    return true;
}
