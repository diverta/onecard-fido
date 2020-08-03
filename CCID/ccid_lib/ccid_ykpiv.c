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
#include "ccid_ykpiv.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

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

    // for debug
    fido_log_debug("Management key (%d bytes)", CAADM_KEY_SIZE);
    fido_log_print_hexdump_debug(key, CAADM_KEY_SIZE);

    // 正常終了
    return SW_NO_ERROR;
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
