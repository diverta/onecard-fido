/* 
 * File:   ccid_openpgp.c
 * Author: makmorit
 *
 * Created on 2021/02/08, 15:43
 */
#include <string.h>

#include "ccid_openpgp.h"

#define MAX_PIN_LENGTH              64
#define PW_STATUS_LENGTH            7
#define PW_RETRY_COUNTER_DEFAULT    3

//
// offset
//  0: aid
//  6: version
//  8: manufacturer
// 10: serial number
//
static const uint8_t aid[] = {0xD2, 0x76, 0x00, 0x01, 0x24, 0x01, 0x03, 0x04, 0xf1, 0xd0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

//
// offset
//  1: card services
//  2: Section 6.2
//  3: card capabilities
//  4: full/partial
//  5: data coding byte
//  6: extended apdu (Section 6.1)
//
static const uint8_t historical_bytes[] = {0x00, 0x31, 0xC5, 0x73, 0xC0, 0x01, 0x40, 0x05, 0x90, 0x00};

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

static uint16_t pin_get_retries(uint8_t *retries_pw1, uint8_t *retries_pw3, uint8_t *retries_rc)
{
    // TODO: リトライカウンターをFlash ROMから読出し
    *retries_pw1 = PW_RETRY_COUNTER_DEFAULT;
    *retries_pw3 = PW_RETRY_COUNTER_DEFAULT;
    *retries_rc = PW_RETRY_COUNTER_DEFAULT;
    return SW_NO_ERROR;
}

static uint16_t set_pw_status(command_apdu_t *capdu, response_apdu_t *rapdu)
{
    uint8_t *rdata = rapdu->data;

    // TODO: PW statusの先頭バイトを、Flash ROMから読出し
    // if (read_attr(DATA_PATH, TAG_PW_STATUS, rdata, 1) < 0) return SW_FILE_NOT_FOUND;

    // TODO: リトライカウンターをFlash ROMから読出し
    uint8_t retries_pw1, retries_rc, retries_pw3;
    uint16_t ret = pin_get_retries(&retries_pw1, &retries_rc, &retries_pw3);
    if (ret != SW_NO_ERROR) {
        return ret;
    }

    // レスポンスを設定
    rdata[1] = MAX_PIN_LENGTH;
    rdata[2] = MAX_PIN_LENGTH;
    rdata[3] = MAX_PIN_LENGTH;
    rdata[4] = retries_pw1;
    rdata[5] = retries_rc;
    rdata[6] = retries_pw3;
    rapdu->len = PW_STATUS_LENGTH;

    return SW_NO_ERROR;
}

static uint16_t openpgp_ins_get_data(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // パラメーターのチェック
    if (capdu->lc != 0) {
        return SW_WRONG_LENGTH;
    }

    uint16_t tag = (uint16_t)(capdu->p1 << 8u) | capdu->p2;
    switch (tag) {
        case TAG_AID:
            memcpy(rapdu->data, aid, sizeof(aid));
            // TODO: シリアル番号を埋める
            // fill_sn(rapdu->data + 10);
            rapdu->len = sizeof(aid);
            break;
        case TAG_HISTORICAL_BYTES:
            memcpy(rapdu->data, historical_bytes, sizeof(historical_bytes));
            rapdu->len = sizeof(historical_bytes);
            break;
        case TAG_PW_STATUS:
            return set_pw_status(capdu, rapdu);
        default:
            return SW_REFERENCE_DATA_NOT_FOUND;
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
        case OPENPGP_INS_GET_DATA:
            rapdu->sw = openpgp_ins_get_data(capdu, rapdu);
            break;
        default:
            rapdu->sw = SW_INS_NOT_SUPPORTED;
            break;
    }
}

void ccid_openpgp_stop_applet(void)
{
}
