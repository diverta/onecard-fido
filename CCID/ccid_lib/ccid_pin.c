/* 
 * File:   ccid_pin.c
 * Author: makmorit
 *
 * Created on 2020/07/22, 10:47
 */
#include <string.h>

#include "ccid_pin.h"

// リトライカウンター
static uint16_t default_retry_counter = PIN_DEFAULT_RETRY_CNT;
static uint16_t retry_counter = PIN_DEFAULT_RETRY_CNT;

// PINコードを保持
static uint8_t m_pin_buffer[PIN_DEFAULT_SIZE];

// 一時読込み用領域
static uint8_t pin_buf[PIN_DEFAULT_SIZE];

//
// 仮の実装です。
//
#define RETRY_ATTR 0
#define DEFAULT_RETRY_ATTR 1
static int read_attr(const char *path, uint8_t attr, void *buf, size_t len) 
{
    if (attr == DEFAULT_RETRY_ATTR) {
        memcpy(buf, &default_retry_counter, sizeof(default_retry_counter));
    } else {
        memcpy(buf, &retry_counter, sizeof(retry_counter));
    }
    return 0;
}
static int write_attr(const char *path, uint8_t attr, const void *buf, size_t len) 
{
    if (attr == RETRY_ATTR) {
        memcpy(&retry_counter, buf, sizeof(retry_counter));
    }
    return 0;
}
static int read_file(void *buf, size_t len) 
{
    memset(m_pin_buffer, 0xff, sizeof(m_pin_buffer));
    memcpy(buf, m_pin_buffer, len);
    return len;
}
static int write_file(const char *path, const void *buf, size_t off, size_t len, uint8_t trunc) 
{
    memset(m_pin_buffer, 0xff, sizeof(m_pin_buffer));
    memcpy(m_pin_buffer, buf, len);
    return 0;
}

static bool restore_pin_code(uint8_t *pin_buf, size_t pin_buf_size)
{
    // 登録されているPINを取得
    if (read_file(pin_buf, pin_buf_size) < 0) {
        return false;
    }
    return true;
}

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

    // リトライカウンターの現在値を参照
    uint8_t ctr;
    int err = read_attr(NULL, RETRY_ATTR, &ctr, sizeof(ctr));
    if (err < 0) {
        return false;
    }
    *retries = ctr;

    // リトライカウンターが０であれば認証失敗
    if (ctr == 0) {
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
        --ctr;
        *retries = ctr;
        // 内部で保持しているリトライカウンターを更新
        err = write_attr(NULL, RETRY_ATTR, &ctr, sizeof(ctr));
        if (err < 0) {
            return false;
        }
        *auth_failed = true;
        return true;
    }

    // 認証が成功したら、リトライカウンターをデフォルト値に再設定
    err = read_attr(NULL, DEFAULT_RETRY_ATTR, &ctr, sizeof(ctr));
    if (err < 0) {
        return false;
    }
    err = write_attr(NULL, RETRY_ATTR, &ctr, sizeof(ctr));
    if (err < 0) {
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

    // リトライカウンターの現在値を参照
    int err = read_attr(NULL, RETRY_ATTR, retries, sizeof(uint8_t));
    if (err < 0) {
        return false;
    }

    return true;
}

//
// 仮の実装です。
//
bool ccid_pin_create(const void *buf, uint8_t len, uint8_t max_retries) 
{
    // PINを設定
    int err = write_file(NULL, buf, 0, len, 1);
    if (err < 0) {
        return false;
    }

    // 内部保持用のリトライカウンターを設定
    err = write_attr(NULL, RETRY_ATTR, &max_retries, sizeof(max_retries));
    if (err < 0) {
        return false;
    }

    // デフォルトのリトライカウンターを設定
    err = write_attr(NULL, DEFAULT_RETRY_ATTR, &max_retries, sizeof(max_retries));
    if (err < 0) {
        return false;
    }

    return true;
}
