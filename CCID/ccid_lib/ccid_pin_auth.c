/* 
 * File:   ccid_pin_auth.c
 * Author: makmorit
 *
 * Created on 2021/02/11, 9:43
 */
#include <string.h>

#include "ccid_apdu.h"
#include "ccid_openpgp_object.h"
#include "ccid_pin.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// PIN種別情報
//
static PIN_T pw1 = {.type = OPGP_PIN_PW1, .size_min = 6, .size_max = 64, .is_validated = false, .current_retries = 3, .default_retries = 3, .default_code = "123456"};
static PIN_T pw3 = {.type = OPGP_PIN_PW3, .size_min = 8, .size_max = 64, .is_validated = false, .current_retries = 3, .default_retries = 3, .default_code = "12345678"};
static PIN_T rc  = {.type = OPGP_PIN_RC,  .size_min = 8, .size_max = 64, .is_validated = false, .current_retries = 3, .default_retries = 3, .default_code = "12345678"};

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
// PINオブジェクト項目参照
//
static uint8_t *stored_pin;
static uint8_t  stored_pin_size;
static uint8_t  stored_retries;

static uint16_t ccid_pin_func_terminate(uint16_t sw)
{
    // PINオブジェクトの一時読み込み用領域をクリア
    ccid_openpgp_object_pin_clear();
    return sw;
}

//
// 関数群
//
static bool restore_pin_object(PIN_T *pin)
{
    // パラメーターチェック
    if (pin == NULL) {
        return false;
    }

    // 登録されているリトライカウンターを取得
    if (ccid_openpgp_object_pin_get(pin, &stored_pin, &stored_pin_size, &stored_retries) == false) {
        return false;
    }

#if LOG_DEBUG_PIN_OBJECT
    fido_log_debug("PIN object data (type=%d, retries=%d): ", pin->type, stored_retries);
    fido_log_print_hexdump_debug(stored_pin, stored_pin_size);
#endif

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
    // 認証済みフラグ／現在リトライカウンターを初期化
    pin->is_validated = false;
    pin->current_retries = 0;

    // パラメーターチェック
    if (pin == NULL || buf == NULL) {
        return SW_UNABLE_TO_PROCESS;
    }
    if (len < pin->size_min || len > pin->size_max) {
        return SW_WRONG_LENGTH;
    }

    // Flash ROMのPINオブジェクトを参照
    if (restore_pin_object(pin) == false) {
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
        pin->current_retries = (--current_cnt);
    } else {
        // OKの場合はリトライカウンターをデフォルトに設定
        pin->current_retries = pin->default_retries;
        pin->is_validated = true;
    }
    return ccid_pin_func_terminate(SW_NO_ERROR);
}

uint16_t ccid_pin_auth_get_retries(PIN_T *pin) 
{
    // Flash ROMのPINオブジェクトを参照
    if (restore_pin_object(pin) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // PINコードがブランクであればゼロクリア
    bool is_blank;
    if (pin_code_is_blank(&is_blank) == false) {
        return ccid_pin_func_terminate(SW_UNABLE_TO_PROCESS);
    }
    if (is_blank) {
        // リトライカウンターの現在値を０に設定
        pin->current_retries = 0;
        return ccid_pin_func_terminate(SW_NO_ERROR);
    }

    // リトライカウンターの現在値を設定して戻す
    pin->current_retries = stored_retries;
    return ccid_pin_func_terminate(SW_NO_ERROR);
}

uint16_t ccid_pin_auth_update_retries(PIN_T *pin)
{
    // Flash ROMのPINオブジェクトを参照
    // （登録されていない場合はデフォルトが戻ります）
    if (restore_pin_object(pin) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // PIN／リトライカウンターをセットで登録
    if (ccid_openpgp_object_pin_set(pin, stored_pin, stored_pin_size, pin->current_retries) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // 一時読込領域を初期化して終了
    return ccid_pin_func_terminate(SW_NO_ERROR);
}

