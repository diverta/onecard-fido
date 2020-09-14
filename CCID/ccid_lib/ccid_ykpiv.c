/* 
 * File:   ccid_ykpiv.c
 * Author: makmorit
 *
 * Created on 2020/08/03, 12:16
 */
#include <string.h>

#include "ccid.h"
#include "ccid_piv.h"
#include "ccid_piv_authenticate.h"
#include "ccid_piv_object.h"
#include "ccid_ykpiv_import_key.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

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

uint16_t ccid_ykpiv_ins_set_mgmkey(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // パラメーターのチェック
    if (capdu->p1 != 0xff || capdu->p2 != 0xff) {
        return SW_WRONG_P1P2;
    }
    if (capdu->lc != CAADM_KEY_SIZE + 3) {
        return SW_WRONG_LENGTH;
    }
    uint8_t *cdata = capdu->data;
    if (cdata[0] != 0x03 || cdata[1] != TAG_KEY_CAADM || cdata[2] != CAADM_KEY_SIZE) {
        return SW_WRONG_LENGTH;
    }

    // 管理コマンドが実行可能でない場合は終了
    if (ccid_piv_admin_mode_get() == false) {
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }

    // パスワードを登録
    uint8_t *key = cdata + 3;
    if (ccid_flash_piv_object_card_admin_key_write(key, CAADM_KEY_SIZE, ALG_TDEA_3KEY) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // Flash ROM書込みが完了するまで、レスポンスを抑止
    apdu_resume_prepare(capdu, rapdu);

    // 正常終了
    return SW_NO_ERROR;
}

void ccid_ykpiv_ins_set_mgmkey_retry(void)
{
    ASSERT(m_capdu);
    ASSERT(m_rapdu);

    // リトライが必要な場合は
    // パスワード登録処理を再実行
    uint16_t sw = ccid_ykpiv_ins_set_mgmkey(m_capdu, m_rapdu);
    if (sw == SW_NO_ERROR) {
        // 正常時は、Flash ROM書込みが完了するまで、レスポンスを抑止
        fido_log_warning("Card administration key registration retry");
    } else {
        // 異常時はエラーレスポンス処理を指示
        fido_log_error("Card administration key registration retry fail");
        apdu_resume_process(m_capdu, m_rapdu, sw);        
    }
}

void ccid_ykpiv_ins_set_mgmkey_resume(bool success)
{
    ASSERT(m_capdu);
    ASSERT(m_rapdu);

    if (success) {
        // Flash ROM書込みが完了した場合は正常レスポンス処理を指示
        fido_log_info("Card administration key registration success");
        apdu_resume_process(m_capdu, m_rapdu, SW_NO_ERROR);
    } else {
        // Flash ROM書込みが失敗した場合はエラーレスポンス処理を指示
        fido_log_error("Card administration key registration fail");
        apdu_resume_process(m_capdu, m_rapdu, SW_UNABLE_TO_PROCESS);
    }
}

uint16_t ccid_ykpiv_ins_import_key(command_apdu_t *capdu, response_apdu_t *rapdu)
{
    // 鍵インポート処理を実行
    uint16_t sw = ccid_ykpiv_import_key(capdu, rapdu);
    if (sw == SW_NO_ERROR) {
        // 正常時は、Flash ROM書込みが完了するまで、レスポンスを抑止
        apdu_resume_prepare(capdu, rapdu);
    }

    // ステータスワードを戻す
    return sw;
}

uint16_t ccid_ykpiv_ins_get_version(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // パラメーターのチェック
    if (capdu->p1 != 0x00 || capdu->p2 != 0x00) {
        return SW_WRONG_P1P2;
    }
    if (capdu->lc != 0) {
        return SW_WRONG_LENGTH;
    }

    // バージョン "5.0.0" を生成
    uint8_t v[] = {0x05, 0x00, 0x00};

    // レスポンスデータを編集
    uint8_t *rdata = rapdu->data;
    rdata[0] = v[0];
    rdata[1] = v[1];
    rdata[2] = v[2];
    rapdu->len = 3;

    // 正常終了
    return SW_NO_ERROR;
}

uint16_t ccid_ykpiv_ins_get_serial(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // パラメーターのチェック
    if (capdu->p1 != 0x00 || capdu->p2 != 0x00) {
        return SW_WRONG_P1P2;
    }
    if (capdu->lc != 0) {
        return SW_WRONG_LENGTH;
    }

    // シリアル "0x00000000" を生成
    uint8_t s[] = {0x00, 0x00, 0x00, 0x00};

    // レスポンスデータを編集
    uint8_t *rdata = rapdu->data;
    memcpy(rdata, s, sizeof(s));
    rapdu->len = sizeof(s);

    // 正常終了
    return SW_NO_ERROR;
}
