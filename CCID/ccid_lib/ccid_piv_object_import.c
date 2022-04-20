/* 
 * File:   ccid_piv_object_import.c
 * Author: makmorit
 *
 * Created on 2020/09/16, 15:28
 */
#include "ccid.h"
#include "ccid_piv.h"
#include "ccid_piv_object.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(ccid_piv_object_import);
#endif

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

static size_t get_enough_space(uint8_t obj_tag) 
{
    switch (obj_tag) {
        case TAG_OBJ_CHUID:
            // Card Holder Unique Identifier
            return MAX_CHUID_SIZE;
        case TAG_CERT_PAUTH:
            // X.509 Certificate for PIV Authentication
            return MAX_CERT_SIZE;
        case TAG_OBJ_CCC:
            // Card Capability Container
            return MAX_CCC_SIZE;
        case TAG_CERT_DGSIG:
            // X.509 Certificate for Digital Signature
            return MAX_CERT_SIZE;
        case TAG_CERT_KEYMN:
            // X.509 Certificate for Key Management
            return MAX_CERT_SIZE;
        default:
            return 0;
    }
}

uint16_t ccid_piv_object_import(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // 管理コマンドが実行可能でない場合は終了
    if (ccid_piv_admin_mode_get() == false) {
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }

    // 受信APDUデータの格納領域
    uint8_t *data = capdu->data;

    // パラメーターのチェック
    if (capdu->p1 != 0x3f || capdu->p2 != 0xff) {
        return SW_WRONG_P1P2;
    }
    if (capdu->lc < 5) {
        return SW_WRONG_LENGTH;
    }
    if (data[0] != 0x5c) {
        return SW_WRONG_DATA;
    }
    if (data[1] != 3 || data[2] != 0x5f || data[3] != 0xc1) {
        // 対象オブジェクトが 0x5fc1xx 形式になっていない場合はエラー
        return SW_FILE_NOT_FOUND;
    }
    uint8_t obj_tag = data[4];
    if (ccid_piv_object_is_obj_tag_exist(obj_tag) == false) {
        return SW_FILE_NOT_FOUND;
    }
    size_t object_size = capdu->lc - 5;
    if (object_size > get_enough_space(obj_tag)) {
        return SW_NOT_ENOUGH_SPACE;
    }

    // PIVオブジェクトデータを登録
    uint8_t *object_data = data + 5;
    if (ccid_flash_piv_object_data_write(obj_tag, object_data, object_size) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // 正常時は、Flash ROM書込みが完了するまで、レスポンスを抑止
    apdu_resume_prepare(capdu, rapdu);
    return SW_NO_ERROR;
}

void ccid_piv_object_import_retry(void)
{
    ccid_assert_apdu(m_capdu, m_rapdu);

    // リトライが必要な場合は
    // 鍵インポート処理を再実行
    uint16_t sw = ccid_piv_object_import(m_capdu, m_rapdu);
    if (sw == SW_NO_ERROR) {
        // 正常時は、Flash ROM書込みが完了するまで、レスポンスを抑止
        fido_log_warning("PIV object data registration retry");
    } else {
        // 異常時はエラーレスポンス処理を指示
        fido_log_error("PIV object data registration retry fail");
        apdu_resume_process(m_capdu, m_rapdu, sw);        
    }
}

void ccid_piv_object_import_resume(bool success)
{
    ccid_assert_apdu(m_capdu, m_rapdu);

    if (success) {
        // Flash ROM書込みが完了した場合は正常レスポンス処理を指示
        fido_log_info("PIV object data registration success");
        apdu_resume_process(m_capdu, m_rapdu, SW_NO_ERROR);
    } else {
        // Flash ROM書込みが失敗した場合はエラーレスポンス処理を指示
        fido_log_error("PIV object data registration fail");
        apdu_resume_process(m_capdu, m_rapdu, SW_UNABLE_TO_PROCESS);
    }
}
