/* 
 * File:   ccid_openpgp.c
 * Author: makmorit
 *
 * Created on 2021/02/08, 15:43
 */
#include <string.h>

#include "ccid_openpgp.h"

//
// offset
//  0: aid
//  6: version
//  8: manufacturer
// 10: serial number
//
static const uint8_t aid[] = {0xD2, 0x76, 0x00, 0x01, 0x24, 0x01, 0x03, 0x04, 0xf1, 0xd0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

bool ccid_openpgp_aid_is_applet(command_apdu_t *capdu)
{
    return (capdu->lc == 6 && memcmp(capdu->data, aid, capdu->lc) == 0);
}

static uint16_t openpgp_ins_select(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // パラメーターのチェック
    if (capdu->p1 != 0x04 || capdu->p2 != 0x00) {
        return SW_WRONG_P1P2;
    }

    // 正常終了
    return SW_NO_ERROR;
}

void ccid_openpgp_apdu_process(command_apdu_t *capdu, response_apdu_t *rapdu)
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
        case OPENPGP_INS_SELECT:
            rapdu->sw = openpgp_ins_select(capdu, rapdu);
            break;
        default:
            rapdu->sw = SW_INS_NOT_SUPPORTED;
            break;
    }
}

void ccid_openpgp_stop_applet(void)
{
}
