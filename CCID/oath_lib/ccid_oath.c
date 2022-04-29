/* 
 * File:   ccid_oath.c
 * Author: makmorit
 *
 * Created on 2022/04/29, 10:42
 */
#include <string.h>

#include "ccid_oath.h"

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
