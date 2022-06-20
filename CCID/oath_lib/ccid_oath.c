/* 
 * File:   ccid_oath.c
 * Author: makmorit
 *
 * Created on 2022/04/29, 10:42
 */
#include <string.h>

#include "ccid_oath.h"
#include "ccid_oath_define.h"
#include "ccid_process.h"
#include "fido_common.h"
#include "rtcc.h"

//
// 時刻同期関連
//
static uint16_t set_current_timestamp_by_totp_counter(uint8_t *secret, uint8_t *challange);

static const uint8_t aid[] = {0xa0, 0x00, 0x00, 0x05, 0x27, 0x21, 0x01};

bool ccid_oath_aid_is_applet(command_apdu_t *capdu)
{
    return (capdu->lc == sizeof(aid) && memcmp(capdu->data, aid, capdu->lc) == 0);
}

void ccid_oath_apdu_process(command_apdu_t *capdu, response_apdu_t *rapdu)
{
    // レスポンス長をゼロクリア
    rapdu->len = 0;

    // CLAのチェック
    if (capdu->cla != 0x00) {
        rapdu->sw = SW_CLA_NOT_SUPPORTED;
        return;
    }

    // INSに応じ処理を分岐
    switch (capdu->ins) {
        default:
            rapdu->sw = SW_INS_NOT_SUPPORTED;
            break;
    }
}

void ccid_oath_stop_applet(void)
{
}

//
// Flash ROM更新後のコールバック関数
//
void ccid_oath_ins_retry(void)
{
    uint16_t sw = SW_NO_ERROR;
    // TODO: 業務処理を再試行
    if (sw == SW_NO_ERROR) {
        // 正常時は、Flash ROM書込みが完了するまで、レスポンスを抑止
        fido_log_warning("OATH account registration retry");
    } else {
        // 異常時はエラーレスポンス処理を指示
        fido_log_error("OATH data object registration retry fail");
        ccid_process_resume_response(sw);        
    }
}

void ccid_oath_ins_resume(bool success)
{
    if (success) {
        // Flash ROM書込みが成功した場合は処理終了
        // TODO: 業務処理を継続実行
        ccid_process_resume_response(SW_NO_ERROR);

    } else {
        // Flash ROM書込みが失敗した場合はエラーレスポンス処理を指示
        fido_log_error("OATH data object registration fail");
        ccid_process_resume_response(SW_UNABLE_TO_PROCESS);
    }
}

// TOTPカウンターを使用し、時刻同期を実行
//
static uint16_t set_current_timestamp_by_totp_counter(uint8_t *secret, uint8_t *challange)
{
    // TOTPでない場合は、何もせず正常終了
    uint8_t alg = secret[0];
    uint8_t oath_type = get_oath_type(alg);
    if (oath_type != OATH_TYPE_TOTP){
        return SW_NO_ERROR;
    }

    // Challangeをカウンター（64ビット整数）に変換
    uint64_t counter = fido_get_uint64_from_bytes(challange);

    // カウンターをRTCCに設定
    if (rtcc_update_timestamp_by_unixtime((uint32_t)counter) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // 正常終了
    return SW_NO_ERROR;
}
