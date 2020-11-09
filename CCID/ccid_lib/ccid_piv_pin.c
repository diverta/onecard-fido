/* 
 * File:   ccid_piv_pin.c
 * Author: makmorit
 *
 * Created on 2020/07/22, 13:05
 */
#include <string.h>

#include "ccid_pin.h"
#include "ccid_piv_object.h"
#include "ccid_piv_pin.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

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
    return true;
}

//
// PIN認証処理
//  入力PIN〜登録PIN間のマッチングの後、
//  リトライカウンター更新が行われます。
//
static bool    m_pin_auth_failed;
static uint8_t m_current_retries;

// Flash ROM書込み時に実行した関数の参照を保持
static void *m_flash_func = NULL;

// APDU格納領域の参照を待避
static command_apdu_t  *m_capdu;
static response_apdu_t *m_rapdu;

static void apdu_resume_prepare(command_apdu_t *capdu, response_apdu_t *rapdu)
{
    // Flash ROM書込みが完了するまで、レスポンスを抑止
    ccid_apdu_response_set_pending(true);

    // APDU格納領域の参照を待避
    m_capdu = capdu;
    m_rapdu = rapdu;
}

static void apdu_resume_process(command_apdu_t *capdu, response_apdu_t *rapdu, uint16_t sw)
{
    // レスポンス処理再開を指示
    rapdu->sw = sw;
    ccid_apdu_response_set_pending(false);
    ccid_apdu_resume_process(capdu, rapdu);
}

static bool save_pin_retries_current(uint8_t retries)
{
    // 登録されているPIN／リトライカウンターを取得
    // （登録されていない場合はデフォルトが戻ります）
    uint8_t pin_buf[PIN_DEFAULT_SIZE];
    if (ccid_piv_object_pin_get(TAG_PIV_PIN, pin_buf, NULL) == false) {
        return false;
    }

    // PIN／リトライカウンターをセットで登録
    return ccid_piv_object_pin_set(TAG_PIV_PIN, pin_buf, retries);
}

static uint16_t verify_pin_code(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // 入力されたPINで認証実行
    uint8_t *cdata = capdu->data;
    pin_is_validated = false;
    if (ccid_pin_verify(cdata, PIN_DEFAULT_SIZE, &m_current_retries, &m_pin_auth_failed) == false) {
        // 登録されたPINが照会できない場合は処理失敗
        return SW_UNABLE_TO_PROCESS;
    }
    if (m_current_retries == 0) {
        // リトライカウンターが0であれば認証をブロック
        return SW_AUTHENTICATION_BLOCKED;
    }

    // 現在のリトライカウンターを更新
    //  Flash ROMを更新後、
    //  ccid_piv_pin_retry／ccid_piv_pin_resumeの
    //  いずれかがコールバックされます。
    if (save_pin_retries_current(m_current_retries)) {
        // 正常時は、Flash ROM書込みが完了するまで、レスポンスを抑止
        apdu_resume_prepare(capdu, rapdu);
        return SW_NO_ERROR;

    } else {
        // 異常時はエラーレスポンス処理を指示
        return SW_UNABLE_TO_PROCESS;
    }
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
    uint16_t ret = verify_pin_code(capdu, rapdu);
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
    m_flash_func = (void *)ccid_piv_pin_auth;
    return verify_pin_code(capdu, rapdu);
}

static void ccid_piv_pin_auth_resume(command_apdu_t *capdu, response_apdu_t *rapdu)
{
    if (m_pin_auth_failed) {
        // 認証NGの場合は、現在のリトライカウンターを戻す
        fido_log_error("PIV PIN verification fail");
        apdu_resume_process(capdu, rapdu, SW_PIN_RETRIES + m_current_retries);
        return;
    }

    // 処理が正常終了
    pin_is_validated = true;
    fido_log_debug("PIV PIN verification success");
    apdu_resume_process(capdu, rapdu, SW_NO_ERROR);
}

//
// Flash ROM更新後のコールバック関数
//
void ccid_piv_pin_retry(void)
{
    ASSERT(m_capdu);
    ASSERT(m_rapdu);

    // リトライが必要な場合は
    // 呼び出し先に応じて、処理を再実行
    uint16_t sw = SW_NO_ERROR;
    if (m_flash_func == (void *)ccid_piv_pin_auth) {
        // 入力されたPINで認証実行
        sw = verify_pin_code(m_capdu, m_rapdu);
    }

    if (sw == SW_NO_ERROR) {
        // 正常時は、Flash ROM書込みが完了するまで、レスポンスを抑止
        fido_log_warning("PIV PIN data registration retry");
    } else {
        // 異常時はエラーレスポンス処理を指示
        fido_log_error("PIV PIN data registration retry fail");
        apdu_resume_process(m_capdu, m_rapdu, sw);        
    }
}

void ccid_piv_pin_resume(bool success)
{
    ASSERT(m_capdu);
    ASSERT(m_rapdu);

    if (success == false) {
        // Flash ROM書込みが失敗した場合はエラーレスポンス処理を指示
        fido_log_error("PIV PIN data registration fail");
        apdu_resume_process(m_capdu, m_rapdu, SW_UNABLE_TO_PROCESS);
        return;
    }

    // Flash ROM書込みが完了した場合は
    // 正常系の後続処理を実行
    if (m_flash_func == (void *)ccid_piv_pin_auth) {
        // PIN認証完了
        ccid_piv_pin_auth_resume(m_capdu, m_rapdu);
    }
}
