/* 
 * File:   ccid_openpgp_data.c
 * Author: makmorit
 *
 * Created on 2021/02/16, 10:37
 */
#include "ccid_openpgp.h"
#include "ccid_openpgp_attr.h"
#include "ccid_openpgp_data.h"
#include "ccid_openpgp_key.h"
#include "ccid_openpgp_key_rsa.h"
#include "ccid_openpgp_object.h"
#include "ccid_pin.h"
#include "ccid_pin_auth.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(ccid_openpgp_data);
#endif

// Flash ROM書込み時に実行した関数の参照を保持
static void *m_flash_func = NULL;

//
// 初期化処理関連
//
static uint16_t update_attr_terminate(void)
{
    // 無効化フラグをFlash ROMに設定
    uint8_t terminated = 1;
    if (ccid_openpgp_object_data_set(TAG_ATTR_TERMINATED, &terminated, sizeof(terminated)) == false) {
        return SW_UNABLE_TO_PROCESS;
    }
    return SW_NO_ERROR;
}

uint16_t ccid_openpgp_data_terminate(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // パラメーターのチェック
    if (capdu->p1 != 0x00 || capdu->p2 != 0x00) {
        return SW_WRONG_P1P2;
    }

    // リトライカウンター（PW3）チェック
    uint8_t retries_pw3;
    uint16_t sw = ccid_openpgp_attr_get_retries(OPGP_PIN_PW3, &retries_pw3);
    if (sw != SW_NO_ERROR) {
        return sw;
    }

    // 管理機能認証が行われていない場合は終了
    if (retries_pw3 > 0) {
        if (ccid_pin_auth_assert_admin() == false) {
            return SW_SECURITY_STATUS_NOT_SATISFIED;
        }
    }

    // 無効化フラグを設定
    //  Flash ROM更新後、
    //  ccid_openpgp_data_retry または
    //  ccid_openpgp_data_resume のいずれかが
    //  コールバックされます。
    sw = update_attr_terminate();
    if (sw == SW_NO_ERROR) {
        // 正常時は、Flash ROM書込みが完了するまで、レスポンスを抑止
        ccid_openpgp_object_resume_prepare(capdu, rapdu);
        m_flash_func = ccid_openpgp_data_terminate;
    }
    return sw;
}

static uint16_t delete_all_objects(void)
{
    // Flash ROM上の全データオブジェクトを削除
    if (ccid_openpgp_object_data_delete_all() == false) {
        return SW_UNABLE_TO_PROCESS;
    }
    return SW_NO_ERROR;
}

uint16_t ccid_openpgp_data_activate(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // 無効化フラグを読み込み
    uint8_t *terminated;
    if (ccid_openpgp_object_data_get(TAG_ATTR_TERMINATED, &terminated, NULL) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    if (terminated[0] == 1) {
        // 無効化フラグが設定されている場合は、Flash ROM上の全データオブジェクトを削除
        //  Flash ROM更新後、
        //  ccid_openpgp_data_retry または
        //  ccid_openpgp_data_resume のいずれかが
        //  コールバックされます。
        uint16_t sw = delete_all_objects();
        if (sw == SW_NO_ERROR) {
            // 正常時は、Flash ROM書込みが完了するまで、レスポンスを抑止
            ccid_openpgp_object_resume_prepare(capdu, rapdu);
            m_flash_func = ccid_openpgp_data_activate;
        }
    }
    return SW_NO_ERROR;
}

//
// データオブジェクト更新
//
static uint16_t update_pw_status(command_apdu_t *capdu)
{
    // パラメーターチェック
    if (capdu->lc != 1) {
        return SW_WRONG_LENGTH;
    }
    // PWステータスを取得
    uint8_t pw_status = capdu->data[0];
    if (pw_status != 0x00 && pw_status != 0x01) {
        return SW_WRONG_DATA;
    }
    // PWステータスをFlash ROMに設定
    if (ccid_openpgp_object_data_set(TAG_PW_STATUS, &pw_status, sizeof(pw_status)) == false) {
        return SW_UNABLE_TO_PROCESS;
    }
    return SW_NO_ERROR;
}

static uint16_t update_fingerprint(command_apdu_t *capdu, uint16_t tag)
{
    // パラメーターチェック
    if (capdu->lc != KEY_FINGERPRINT_LENGTH) {
        return SW_WRONG_LENGTH;
    }
    // フィンガープリントをFlash ROMに設定
    if (ccid_openpgp_object_data_set(tag, capdu->data, KEY_FINGERPRINT_LENGTH) == false) {
        return SW_UNABLE_TO_PROCESS;
    }
    return SW_NO_ERROR;
}

static uint16_t update_generation_dates(command_apdu_t *capdu, uint16_t tag)
{
    // パラメーターチェック
    if (capdu->lc != KEY_DATETIME_LENGTH) {
        return SW_WRONG_LENGTH;
    }
    // タイムスタンプをFlash ROMに設定
    if (ccid_openpgp_object_data_set(tag, capdu->data, KEY_DATETIME_LENGTH) == false) {
        return SW_UNABLE_TO_PROCESS;
    }
    return SW_NO_ERROR;
}

static uint16_t update_data_object(command_apdu_t *capdu_)
{
    // コマンドAPDU参照を待避
    static command_apdu_t *capdu;
    if (capdu_ != NULL) {
        capdu = capdu_;
    }
    // 例外抑止
    if (capdu == NULL) {
        return SW_UNABLE_TO_PROCESS;
    }

    // データオブジェクトのタグに応じて処理分岐
    uint16_t tag = (uint16_t)(capdu->p1 << 8) | capdu->p2;
    switch (tag) {
        case TAG_PW_STATUS:
            return update_pw_status(capdu);
        case TAG_KEY_SIG_FINGERPRINT:
        case TAG_KEY_DEC_FINGERPRINT:
        case TAG_KEY_AUT_FINGERPRINT:
            return update_fingerprint(capdu, tag);
        case TAG_KEY_SIG_GENERATION_DATES:
        case TAG_KEY_DEC_GENERATION_DATES:
        case TAG_KEY_AUT_GENERATION_DATES:
            return update_generation_dates(capdu, tag);
        default:
            return SW_WRONG_P1P2;
    }
}

uint16_t ccid_openpgp_data_put(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // 管理機能認証が行われていない場合は終了
    if (ccid_pin_auth_assert_admin() == false) {
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }

    // データオブジェクトをFlash ROMに登録
    //  Flash ROM更新後、
    //  ccid_openpgp_data_retry または
    //  ccid_openpgp_data_resume のいずれかが
    //  コールバックされます。
    uint16_t sw = update_data_object(capdu);
    if (sw == SW_NO_ERROR) {
        // 正常時は、Flash ROM書込みが完了するまで、レスポンスを抑止
        ccid_openpgp_object_resume_prepare(capdu, rapdu);
        m_flash_func = ccid_openpgp_data_put;
    }
    return sw;
}

//
// 秘密鍵生成後の鍵／ステータス登録処理
//
static uint16_t m_key_tag;
static uint8_t m_key_status;

static uint16_t register_key(void)
{
    // 例外抑止
    if (m_key_tag == TAG_OPGP_NONE) {
        return SW_UNABLE_TO_PROCESS;
    }
    // 秘密鍵をFlash ROMに登録
    uint8_t *private_key = ccid_openpgp_key_rsa_private_key();
    size_t private_key_size = 512;
    if (ccid_openpgp_object_data_set(m_key_tag, private_key, private_key_size) == false) {
        return SW_UNABLE_TO_PROCESS;
    }
    return SW_NO_ERROR;
}

static uint16_t register_key_status(void)
{
    // 例外抑止
    if (m_key_status == TAG_OPGP_NONE) {
        return SW_UNABLE_TO_PROCESS;
    }
    // 秘密鍵ステータスのデータオブジェクトタグを取得
    uint16_t tag = ccid_openpgp_key_status_tag_get(m_key_tag);
    if (tag == TAG_OPGP_NONE) {
        return SW_WRONG_DATA;
    }
    // 秘密鍵ステータスをFlash ROMに登録
    //  Flash ROM更新後、
    //  ccid_openpgp_data_retry または
    //  ccid_openpgp_data_resume のいずれかが
    //  コールバックされます。
    if (ccid_openpgp_object_data_set(tag, &m_key_status, sizeof(m_key_status)) == false) {
        return SW_UNABLE_TO_PROCESS;
    }
    m_flash_func = register_key_status;
    return SW_NO_ERROR;
}

static uint16_t reset_sign_counter(void)
{
    // 署名カウンターをFlash ROMに登録
    //  Flash ROM更新後、
    //  ccid_openpgp_data_retry または
    //  ccid_openpgp_data_resume のいずれかが
    //  コールバックされます。
    if (openpgp_attr_set_digital_sig_counter(0) == false) {
        return SW_UNABLE_TO_PROCESS;
    }
    m_flash_func = reset_sign_counter;
    return SW_NO_ERROR;
}

uint16_t ccid_openpgp_data_register_key(command_apdu_t *capdu, response_apdu_t *rapdu, uint16_t key_tag, uint8_t key_status) 
{
    // 鍵種別／ステータスを待避
    m_key_tag = key_tag;
    m_key_status = key_status;

    // 秘密鍵をFlash ROMに登録
    //  Flash ROM更新後、
    //  ccid_openpgp_data_retry または
    //  ccid_openpgp_data_resume のいずれかが
    //  コールバックされます。
    uint16_t sw = register_key();
    if (sw == SW_NO_ERROR) {
        // 正常時は、Flash ROM書込みが完了するまで、レスポンスを抑止
        ccid_openpgp_object_resume_prepare(capdu, rapdu);
        m_flash_func = ccid_openpgp_data_register_key;
    }
    return sw;
}

//
// Flash ROM更新後のコールバック関数
//
void ccid_openpgp_data_retry(void)
{
    uint16_t sw = SW_NO_ERROR;
    if (m_flash_func == ccid_openpgp_data_terminate) {
        // 無効化フラグを再度設定
        sw = update_attr_terminate();
    }
    if (m_flash_func == ccid_openpgp_data_activate) {
        // データ全削除を再度実行
        sw = delete_all_objects();
    }
    if (m_flash_func == ccid_openpgp_data_put) {
        // オブジェクトデータ登録を再度実行
        sw = update_data_object(NULL);
    }
    if (m_flash_func == ccid_openpgp_data_register_key) {
        // 秘密鍵データ登録を再度実行
        sw = register_key();
    }
    if (m_flash_func == register_key_status) {
        // 秘密鍵ステータス登録を再度実行
        sw = register_key_status();
    }
    if (m_flash_func == reset_sign_counter) {
        // 署名カウンターリセットを再度実行
        sw = reset_sign_counter();
    }
    if (sw == SW_NO_ERROR) {
        // 正常時は、Flash ROM書込みが完了するまで、レスポンスを抑止
        fido_log_warning("OpenPGP data object registration retry");
    } else {
        // 異常時はエラーレスポンス処理を指示
        fido_log_error("OpenPGP data object registration retry fail");
        ccid_openpgp_object_resume_process(sw);        
    }
}

void ccid_openpgp_data_resume(bool success)
{
    if (success) {
        // Flash ROM書込みが成功した場合は処理終了
        if (m_flash_func == ccid_openpgp_data_terminate) {
            fido_log_info("OpenPGP data object termination success");
        }
        if (m_flash_func == ccid_openpgp_data_activate) {
            fido_log_info("All OpenPGP data object delete success");
        }
        if (m_flash_func == ccid_openpgp_data_put) {
            fido_log_info("OpenPGP data object update success");
        }
        if (m_flash_func == ccid_openpgp_data_register_key) {
            // 秘密鍵データ登録は成功
            fido_log_info("OpenPGP private key register success");
            // 秘密鍵ステータス登録を実行
            uint16_t sw = register_key_status();
            if (sw != SW_NO_ERROR) {
                // 異常時はエラーレスポンス処理を指示
                fido_log_error("OpenPGP private key status update fail");
                ccid_openpgp_object_resume_process(sw);        
            }
            return;
        }
        if (m_flash_func == register_key_status) {
            fido_log_info("OpenPGP private key status update success");
            if (m_key_tag == TAG_KEY_SIG) {
                // 署名カウンター登録を実行
                uint16_t sw = reset_sign_counter();
                if (sw != SW_NO_ERROR) {
                    // 異常時はエラーレスポンス処理を指示
                    fido_log_error("OpenPGP sign counter reset fail");
                    ccid_openpgp_object_resume_process(sw);        
                }
                return;
            }
        }
        if (m_flash_func == reset_sign_counter) {
            fido_log_info("OpenPGP sign counter reset success");
        }
        ccid_openpgp_object_resume_process(SW_NO_ERROR);

    } else {
        // Flash ROM書込みが失敗した場合はエラーレスポンス処理を指示
        fido_log_error("OpenPGP data object registration fail");
        ccid_openpgp_object_resume_process(SW_UNABLE_TO_PROCESS);
    }
}
