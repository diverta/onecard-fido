/* 
 * File:   ccid_ykpiv.c
 * Author: makmorit
 *
 * Created on 2020/08/03, 12:16
 */
#include <string.h>

#include "ccid.h"
#include "ccid_piv.h"
#include "ccid_ykpiv.h"

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
