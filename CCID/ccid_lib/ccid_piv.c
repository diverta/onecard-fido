/* 
 * File:   ccid_piv.c
 * Author: makmorit
 *
 * Created on 2020/06/01, 9:55
 */
#include "ccid.h"
#include "ccid_piv.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

static const uint8_t rid[] = {0xa0, 0x00, 0x00, 0x03, 0x08};
static const uint8_t pix[] = {0x00, 0x00, 0x10, 0x00, 0x01, 0x00};

static uint16_t piv_ins_select(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // パラメーターのチェック
    if (capdu->p1 != 0x04 || capdu->p2 != 0x00) {
        return SW_WRONG_P1P2;
    }

    // レスポンスデータを編集
    uint8_t *rdata = rapdu->data;
    rdata[0] = 0x61;
    rdata[1] = 6 + sizeof(pix) + sizeof(rid);
    rdata[2] = 0x4f;
    rdata[3] = sizeof(pix);
    memcpy(rdata + 4, pix, sizeof(pix));
    rdata[4 + sizeof(pix)] = 0x79;
    rdata[5 + sizeof(pix)] = 2 + sizeof(rid);
    rdata[6 + sizeof(pix)] = 0x4F;
    rdata[7 + sizeof(pix)] = sizeof(rid);
    memcpy(rdata + 8 + sizeof(pix), rid, sizeof(rid));
    rapdu->len = 8 + sizeof(pix) + sizeof(rid);

    // 正常終了
    return SW_NO_ERROR;
}

void ccid_piv_apdu_process(command_apdu_t *capdu, response_apdu_t *rapdu)
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
        case PIV_INS_SELECT:
            rapdu->sw = piv_ins_select(capdu, rapdu);
            break;
        default:
            rapdu->sw = SW_INS_NOT_SUPPORTED;
            break;
    }
}
