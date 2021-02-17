/* 
 * File:   ccid_openpgp_data.c
 * Author: makmorit
 *
 * Created on 2021/02/16, 10:37
 */
#include "ccid_openpgp.h"
#include "ccid_openpgp_attr.h"
#include "ccid_openpgp_data.h"
#include "ccid_openpgp_object.h"
#include "ccid_pin.h"
#include "ccid_pin_auth.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// Flash ROM書込み時に実行した関数の参照を保持
static void *m_flash_func = NULL;

static bool assert_validated_admin(void)
{
    // 管理機能認証が行われているかどうかを戻す
    PIN_T *pw3 = ccid_pin_auth_pin_t(OPGP_PIN_PW3);
    return pw3->is_validated;
}

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
        if (assert_validated_admin() == false) {
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
        default:
            return SW_WRONG_P1P2;
    }
}

uint16_t ccid_openpgp_data_put(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // 管理機能認証が行われていない場合は終了
    if (assert_validated_admin() == false) {
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
        ccid_openpgp_object_resume_process(SW_NO_ERROR);

    } else {
        // Flash ROM書込みが失敗した場合はエラーレスポンス処理を指示
        fido_log_error("OpenPGP data object registration fail");
        ccid_openpgp_object_resume_process(SW_UNABLE_TO_PROCESS);
        return;
    }
}
