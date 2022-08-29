/* 
 * File:   ccid_oath.c
 * Author: makmorit
 *
 * Created on 2022/04/29, 10:42
 */
#include "ccid_oath.h"
#include "ccid_oath_account.h"
#include "ccid_oath_calculate.h"
#include "ccid_oath_define.h"
#include "ccid_oath_list.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(ccid_oath);
#endif

static const uint8_t aid[] = {0xa0, 0x00, 0x00, 0x05, 0x27, 0x21, 0x01};

bool ccid_oath_aid_is_applet(command_apdu_t *capdu)
{
    return (capdu->lc == sizeof(aid) && memcmp(capdu->data, aid, capdu->lc) == 0);
}

static uint16_t oath_ins_select(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // パラメーターのチェック
    if (capdu->p1 != 0x04 || capdu->p2 != 0x00) {
        return SW_WRONG_P1P2;
    }

    // 正常終了
    return SW_NO_ERROR;
}

static uint16_t oath_ins_put(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    return ccid_oath_account_add(capdu, rapdu);
}

static uint16_t oath_ins_delete(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    return ccid_oath_account_delete(capdu, rapdu);
}

static uint16_t oath_ins_reset(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    return ccid_oath_account_reset(capdu, rapdu);
}

static uint16_t oath_ins_calculate(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    return ccid_oath_calculate(capdu, rapdu);
}

static uint16_t oath_ins_list(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    return ccid_oath_list(capdu, rapdu);
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
        case OATH_INS_SELECT:
            rapdu->sw = oath_ins_select(capdu, rapdu);
            break;
        case OATH_INS_PUT:
            rapdu->sw = oath_ins_put(capdu, rapdu);
            break;
        case OATH_INS_DELETE:
            rapdu->sw = oath_ins_delete(capdu, rapdu);
            break;
        case OATH_INS_RESET:
            rapdu->sw = oath_ins_reset(capdu, rapdu);
            break;
        case OATH_INS_CALCULATE:
            rapdu->sw = oath_ins_calculate(capdu, rapdu);
            break;
        case OATH_INS_LIST:
            rapdu->sw = oath_ins_list(capdu, rapdu);
            break;
        default:
            rapdu->sw = SW_INS_NOT_SUPPORTED;
            break;
    }
}

void ccid_oath_stop_applet(void)
{
    fido_log_debug("Applet OATH stopped");
}
