/* 
 * File:   ccid_openpgp_pin.c
 * Author: makmorit
 *
 * Created on 2021/02/10, 17:17
 */
#include "ccid_openpgp.h"
#include "ccid_openpgp_pin.h"
#include "ccid_openpgp_object.h"
#include "ccid_pin_auth.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// Flash ROM書込み時に実行した関数の参照を保持
static void *m_flash_func = NULL;

// PIN格納領域、PIN長を保持
static uint8_t *m_new_pin;
static uint8_t  m_new_pin_size;

//
// PIN種別／認証モードを保持
//
static PIN_T  *m_pw;
static uint8_t m_pw1_mode;

void ccid_openpgp_pin_pw1_mode81_set(bool b)
{
    if (b) {
        m_pw1_mode |= 0x01;
    } else {
        m_pw1_mode &= 0xfe;
    }
}

void ccid_openpgp_pin_pw1_mode82_set(bool b)
{
    if (b) {
        m_pw1_mode |= 0x02;
    } else {
        m_pw1_mode &= 0xfd;
    }
}

uint8_t ccid_openpgp_pin_pw1_mode81_get(void)
{
    return m_pw1_mode & 0x01;
}

uint8_t ccid_openpgp_pin_pw1_mode82_get(void)
{
    return m_pw1_mode & 0x02;
}

void ccid_openpgp_pin_pw1_mode_clear(void)
{
    m_pw1_mode = 0x00;
}

void ccid_openpgp_pin_pw_clear_validated(void)
{
    PIN_T *pw1 = ccid_pin_auth_pin_t(OPGP_PIN_PW1);
    pw1->is_validated = false;

    PIN_T *pw3 = ccid_pin_auth_pin_t(OPGP_PIN_PW3);
    pw3->is_validated = false;
}

uint16_t ccid_openpgp_pin_auth(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // パラメーターのチェック
    if (capdu->p1 != 0x00 && capdu->p1 != 0xff) {
        return SW_WRONG_P1P2;
    }

    // PIN種別判定／認証済みフラグをクリア
    if (capdu->p2 == 0x81) {
        m_pw = ccid_pin_auth_pin_t(OPGP_PIN_PW1);
        ccid_openpgp_pin_pw1_mode81_set(false);
    } else if (capdu->p2 == 0x82) {
        m_pw = ccid_pin_auth_pin_t(OPGP_PIN_PW1);
        ccid_openpgp_pin_pw1_mode82_set(false);
    } else if (capdu->p2 == 0x83) {
        m_pw = ccid_pin_auth_pin_t(OPGP_PIN_PW3);
    } else {
        return SW_WRONG_P1P2;
    }

    // PIN認証クリアの場合
    if (capdu->p1 == 0xff) {
        m_pw->is_validated = false;
        return SW_NO_ERROR;
    }

    // PINリトライカウンター照会の場合
    if (capdu->lc == 0) {
        if (m_pw->is_validated) {
            return SW_NO_ERROR;
        }
        // Flash ROMに登録されているリトライカウンターを応答
        uint16_t sw = ccid_pin_auth_get_retries(m_pw);
        if (sw != SW_NO_ERROR) {
            return sw;
        } else {
            return SW_PIN_RETRIES + m_pw->current_retries;
        }
    }

    // 入力PINコードで認証
    uint16_t sw = ccid_pin_auth_verify(m_pw, capdu->data, capdu->lc);
    if (sw != SW_NO_ERROR) {
        return sw;
    }

    // 認証済みフラグを設定
    if (capdu->p2 == 0x81) {
        ccid_openpgp_pin_pw1_mode81_set(true);
    } else if (capdu->p2 == 0x82) {
        ccid_openpgp_pin_pw1_mode82_set(true);
    }

    // 現在のリトライカウンターを更新
    //  Flash ROM更新後、
    //  ccid_openpgp_pin_retry または
    //  ccid_openpgp_pin_resume のいずれかが
    //  コールバックされます。
    sw = ccid_pin_auth_update_retries(m_pw);
    if (sw == SW_NO_ERROR) {
        // 正常時は、Flash ROM書込みが完了するまで、レスポンスを抑止
        ccid_openpgp_object_resume_prepare(capdu, rapdu);
        m_flash_func = ccid_openpgp_pin_auth;
    }
    return sw;
}

static void ccid_openpgp_pin_auth_resume(void)
{
    if (m_pw->is_validated == false) {
        // ccid_pin_auth_verify において
        // 認証NGの場合は、現在のリトライカウンターを戻す
        fido_log_error("OpenPGP PIN verification fail");
        ccid_openpgp_object_resume_process(SW_PIN_RETRIES + m_pw->current_retries);
        return;
    }

    // 処理が正常終了
    fido_log_info("OpenPGP PIN verification success");
    ccid_openpgp_object_resume_process(SW_NO_ERROR);
}

uint16_t ccid_openpgp_pin_update(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // パラメーターのチェック
    if (capdu->p1 != 0x00) {
        return SW_WRONG_P1P2;
    }

    // PIN種別判定／認証済みフラグをクリア
    if (capdu->p2 == 0x81) {
        m_pw = ccid_pin_auth_pin_t(OPGP_PIN_PW1);
        ccid_openpgp_pin_pw1_mode_clear();
    } else if (capdu->p2 == 0x83) {
        m_pw = ccid_pin_auth_pin_t(OPGP_PIN_PW3);
    } else {
        return SW_WRONG_P1P2;
    }

    // 現在登録されているPINの長さを取得
    uint8_t pin_size;
    uint16_t sw = ccid_pin_auth_get_code_size(m_pw, &pin_size);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    
    // 入力PINコードで認証
    sw = ccid_pin_auth_verify(m_pw, capdu->data, pin_size);
    if (sw != SW_NO_ERROR) {
        return sw;
    }

    if (m_pw->is_validated == false) {
        // 認証NGの場合は、現在のリトライカウンターを更新
        sw = ccid_pin_auth_update_retries(m_pw);

    } else {
        // PIN格納領域、PIN長を保持
        m_new_pin = capdu->data + pin_size;
        m_new_pin_size = capdu->lc - pin_size;

        // 認証OKの場合は、PIN番号を更新し、
        // リトライカウンターをデフォルト値に設定
        sw = ccid_pin_auth_update_code(m_pw, m_new_pin, m_new_pin_size);
    }

    if (sw == SW_NO_ERROR) {
        // 正常時は、Flash ROM書込みが完了するまで、レスポンスを抑止
        //  Flash ROM更新後、
        //  ccid_openpgp_pin_retry または
        //  ccid_openpgp_pin_resume のいずれかが
        //  コールバックされます。
        ccid_openpgp_object_resume_prepare(capdu, rapdu);
        m_flash_func = ccid_openpgp_pin_update;
    }
    return sw;
}

static void ccid_openpgp_pin_update_resume(void)
{
    if (m_pw->is_validated == false) {
        // ccid_pin_auth_verify において
        // 認証NGの場合は、現在のリトライカウンターを戻す
        fido_log_error("OpenPGP PIN update fail");
        ccid_openpgp_object_resume_process(SW_PIN_RETRIES + m_pw->current_retries);
        return;
    }

    // 処理が正常終了
    fido_log_info("OpenPGP PIN update success");
    ccid_openpgp_object_resume_process(SW_NO_ERROR);
}

static uint16_t auth_with_resetting_code(command_apdu_t *capdu, response_apdu_t *rapdu, uint8_t reset_code_size)
{
    // リセットコードで認証
    PIN_T *reset_code = ccid_pin_auth_pin_t(OPGP_PIN_RC);
    uint16_t sw = ccid_pin_auth_verify(reset_code, capdu->data, reset_code_size);
    if (sw != SW_NO_ERROR) {
        return sw;
    }

    // 現在のリトライカウンターを更新
    //  Flash ROM更新後、
    //  ccid_openpgp_pin_retry または
    //  ccid_openpgp_pin_resume のいずれかが
    //  コールバックされます。
    m_pw = reset_code;
    sw = ccid_pin_auth_update_retries(m_pw);
    if (sw == SW_NO_ERROR) {
        // 正常時は、Flash ROM書込みが完了するまで、レスポンスを抑止
        ccid_openpgp_object_resume_prepare(capdu, rapdu);
        m_flash_func = auth_with_resetting_code;
    }
    return sw;
}

static void auth_with_resetting_code_resume(void)
{
    if (m_pw->is_validated == false) {
        // 認証NGの場合は、現在のリトライカウンターを戻す
        fido_log_error("OpenPGP auth with resetting code fail");
        ccid_openpgp_object_resume_process(SW_PIN_RETRIES + m_pw->current_retries);
        return;
    }

    // 認証OKの場合
    fido_log_info("OpenPGP auth with resetting code success");

    // PIN番号を更新し、リトライカウンターをデフォルト値に設定
    //  Flash ROM更新後、
    //  ccid_openpgp_pin_retry または
    //  ccid_openpgp_pin_resume のいずれかが
    //  コールバックされます。
    m_pw = ccid_pin_auth_pin_t(OPGP_PIN_PW1);
    uint16_t sw = ccid_pin_auth_update_code(m_pw, m_new_pin, m_new_pin_size);
    if (sw == SW_NO_ERROR) {
        // 正常時は、Flash ROM書込みが完了するまで、レスポンスを抑止
        m_flash_func = ccid_openpgp_pin_reset;

    } else {
        // 異常時は、レスポンスを戻すよう指示
        ccid_openpgp_object_resume_process(sw);
    }
}

uint16_t ccid_openpgp_pin_reset(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // パラメーターのチェック
    if ((capdu->p1 != 0x00 && capdu->p1 != 0x02) || capdu->p2 != 0x81) {
        return SW_WRONG_P1P2;
    }
    
    uint16_t sw;
    if (capdu->p1 == 0x00) {
        // リセットコード認証とPINリセットを同時実行する場合
        // 現在登録されているリセットコードの長さを取得
        PIN_T *reset_code = ccid_pin_auth_pin_t(OPGP_PIN_RC);
        uint8_t reset_code_size;
        sw = ccid_pin_auth_get_code_size(reset_code, &reset_code_size);
        if (sw != SW_NO_ERROR) {
            return sw;
        }

        // PIN格納領域、PIN長を保持
        m_new_pin = capdu->data + reset_code_size;
        m_new_pin_size = capdu->lc - reset_code_size;

        // リセットコードで認証（リトライカウンターの更新あり）
        sw = auth_with_resetting_code(capdu, rapdu, reset_code_size);

    } else {
        // 管理用PINによる認証が行われていない場合は終了
        if (ccid_pin_auth_assert_admin() == false) {
            return SW_SECURITY_STATUS_NOT_SATISFIED;
        }

        // PIN格納領域、PIN長を保持
        m_new_pin = capdu->data;
        m_new_pin_size = capdu->lc;

        // PIN番号を更新し、リトライカウンターをデフォルト値に設定
        //  Flash ROM更新後、
        //  ccid_openpgp_pin_retry または
        //  ccid_openpgp_pin_resume のいずれかが
        //  コールバックされます。
        m_pw = ccid_pin_auth_pin_t(OPGP_PIN_PW1);
        sw = ccid_pin_auth_update_code(m_pw, m_new_pin, m_new_pin_size);
        if (sw == SW_NO_ERROR) {
            // 正常時は、Flash ROM書込みが完了するまで、レスポンスを抑止
            ccid_openpgp_object_resume_prepare(capdu, rapdu);
            m_flash_func = ccid_openpgp_pin_reset;
        }
    }
    return sw;
}

static void ccid_openpgp_pin_reset_resume(void)
{
    // 処理が正常終了
    fido_log_info("OpenPGP PIN reset success");
    ccid_openpgp_object_resume_process(SW_NO_ERROR);
}

uint16_t ccid_openpgp_pin_update_reset_code(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // TODO: 仮の実装です。
    return SW_INS_NOT_SUPPORTED;
}

//
// Flash ROM更新後のコールバック関数
//
void ccid_openpgp_pin_retry(void)
{
    uint16_t sw = SW_NO_ERROR;
    if (m_flash_func == ccid_openpgp_pin_auth) {
        // 現在のリトライカウンターを再度更新
        sw = ccid_pin_auth_update_retries(m_pw);

    } else if (m_flash_func == ccid_openpgp_pin_update) {
        if (m_pw->is_validated == false) {
            // 現在のリトライカウンターを再度更新
            sw = ccid_pin_auth_update_retries(m_pw);
        } else {
            // PIN番号更新を再度実行
            sw = ccid_pin_auth_update_code(m_pw, m_new_pin, m_new_pin_size);
        }

    } else if (m_flash_func == auth_with_resetting_code) {
        // リセットコードのリトライカウンターを再度更新
        sw = ccid_pin_auth_update_retries(m_pw);

    } else if (m_flash_func == ccid_openpgp_pin_reset) {
        // PIN番号更新を再度実行
        sw = ccid_pin_auth_update_code(m_pw, m_new_pin, m_new_pin_size);
    }

    if (sw == SW_NO_ERROR) {
        // 正常時は、Flash ROM書込みが完了するまで、レスポンスを抑止
        fido_log_warning("OpenPGP PIN data registration retry");
    } else {
        // 異常時はエラーレスポンス処理を指示
        fido_log_error("OpenPGP PIN data registration retry fail");
        ccid_openpgp_object_resume_process(sw);        
    }
}

void ccid_openpgp_pin_resume(bool success)
{
    if (success) {
        if (m_flash_func == ccid_openpgp_pin_auth) {
            // Flash ROM書込みが成功した場合はPIN認証完了
            ccid_openpgp_pin_auth_resume();

        } else if (m_flash_func == ccid_openpgp_pin_update) {
            // Flash ROM書込みが成功した場合はPIN番号更新完了
            ccid_openpgp_pin_update_resume();

        } else if (m_flash_func == auth_with_resetting_code) {
            // Flash ROM書込みが成功した場合はリセットコード認証完了
            auth_with_resetting_code_resume();

        } else if (m_flash_func == ccid_openpgp_pin_reset) {
            // Flash ROM書込みが成功した場合はPINリセット完了
            ccid_openpgp_pin_reset_resume();
        }

    } else {
        // Flash ROM書込みが失敗した場合はエラーレスポンス処理を指示
        fido_log_error("OpenPGP PIN data registration fail");
        ccid_openpgp_object_resume_process(SW_UNABLE_TO_PROCESS);
    }
}
