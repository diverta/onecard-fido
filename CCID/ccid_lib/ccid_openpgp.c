/* 
 * File:   ccid_openpgp.c
 * Author: makmorit
 *
 * Created on 2021/02/08, 15:43
 */
#include <string.h>

#include "ccid_openpgp.h"
#include "ccid_openpgp_attr.h"
#include "ccid_openpgp_data.h"
#include "ccid_openpgp_key.h"
#include "ccid_openpgp_pin.h"
#include "ccid_pin_auth.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

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

static const uint8_t extended_length_info[] = {
    0x02, 0x02, HI(APDU_BUFFER_SIZE), LO(APDU_BUFFER_SIZE),
    0x02, 0x02, HI(APDU_BUFFER_SIZE), LO(APDU_BUFFER_SIZE)
};

//
// offset
//  0: Support key import, pw1 status change, and algorithm attributes changes
//  1: No SM algorithm
//  3: No challenge support
//  4: Cert length
//  6: Other DO length
//  8: No PIN block 2 format
//  9: No MSE
//
static const uint8_t extended_capabilities[] = {
    0x34, 0x00, 0x00, 0x00, 
    HI(OPGP_MAX_CERT_LENGTH), LO(OPGP_MAX_CERT_LENGTH),
    HI(OPGP_MAX_DO_LENGTH), LO(OPGP_MAX_DO_LENGTH),
    0x00, 0x00};

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

static size_t get_aid_bytes(uint8_t *buff)
{
    memcpy(buff, aid, sizeof(aid));
    // TODO: シリアル番号を埋める
    // fill_sn(buff + 10);

    // 設定したAIDの長さを戻す
    return sizeof(aid);
}

static size_t get_historical_bytes(uint8_t *buff)
{
    // 設定したHistorical bytesの長さを戻す
    memcpy(buff, historical_bytes, sizeof(historical_bytes));
    return sizeof(historical_bytes);
}

static uint16_t get_aid(response_apdu_t *rapdu)
{
    rapdu->len = get_aid_bytes(rapdu->data);
    return SW_NO_ERROR;
}

static uint16_t get_historical(response_apdu_t *rapdu)
{
    rapdu->len = get_historical_bytes(rapdu->data);
    return SW_NO_ERROR;
}

static uint16_t get_pw_status(response_apdu_t *rapdu)
{
    size_t size;
    uint16_t sw = openpgp_attr_get_pw_status(rapdu->data, &size);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    rapdu->len = size;
    return SW_NO_ERROR;
}

static uint16_t get_application_related_data(response_apdu_t *rapdu)
{
    uint16_t sw;
    uint8_t offset = 0;
    size_t len = 0;
    uint8_t *rdata = rapdu->data;

    rdata[offset++] = TAG_AID;
    rdata[offset++] = sizeof(aid);
    offset += get_aid_bytes(rdata + offset);

    rdata[offset++] = HI(TAG_HISTORICAL_BYTES);
    rdata[offset++] = LO(TAG_HISTORICAL_BYTES);
    rdata[offset++] = sizeof(historical_bytes);
    offset += get_historical_bytes(rdata + offset);

    rdata[offset++] = HI(TAG_EXTENDED_LENGTH_INFO);
    rdata[offset++] = LO(TAG_EXTENDED_LENGTH_INFO);
    rdata[offset++] = sizeof(extended_length_info);
    memcpy(rdata + offset, extended_length_info, sizeof(extended_length_info));
    offset += sizeof(extended_length_info);

    rdata[offset++] = TAG_DISCRETIONARY_DATA_OBJECTS;
    rdata[offset++] = 0x81;

    uint8_t length_pos = offset;
    rdata[offset++] = 0;

    rdata[offset++] = TAG_EXTENDED_CAPABILITIES;
    rdata[offset++] = sizeof(extended_capabilities);
    memcpy(rdata + offset, extended_capabilities, sizeof(extended_capabilities));
    offset += sizeof(extended_capabilities);

    rdata[offset++] = TAG_ALGORITHM_ATTRIBUTES_SIG;
    sw = openpgp_key_get_attributes(TAG_ALGORITHM_ATTRIBUTES_SIG, rdata + offset + 1, &len);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    rdata[offset++] = len;
    offset += len;

    rdata[offset++] = TAG_ALGORITHM_ATTRIBUTES_DEC;
    sw = openpgp_key_get_attributes(TAG_ALGORITHM_ATTRIBUTES_DEC, rdata + offset + 1, &len);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    rdata[offset++] = len;
    offset += len;

    rdata[offset++] = TAG_ALGORITHM_ATTRIBUTES_AUT;
    sw = openpgp_key_get_attributes(TAG_ALGORITHM_ATTRIBUTES_AUT, rdata + offset + 1, &len);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    rdata[offset++] = len;
    offset += len;

    rdata[offset++] = TAG_PW_STATUS;
    sw = openpgp_attr_get_pw_status(rdata + offset + 1, &len);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    rdata[offset++] = len;
    offset += len;

    rdata[offset++] = TAG_KEY_FINGERPRINTS;
    rdata[offset++] = KEY_FINGERPRINT_LENGTH * 3;
    sw = openpgp_key_get_fingerprint(TAG_KEY_SIG_FINGERPRINT, rdata + offset, &len);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    offset += len;
    sw = openpgp_key_get_fingerprint(TAG_KEY_DEC_FINGERPRINT, rdata + offset, &len);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    offset += len;
    sw = openpgp_key_get_fingerprint(TAG_KEY_AUT_FINGERPRINT, rdata + offset, &len);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    offset += len;

    rdata[offset++] = TAG_CA_FINGERPRINTS;
    rdata[offset++] = KEY_FINGERPRINT_LENGTH * 3;
    sw = openpgp_key_get_fingerprint(TAG_KEY_CA1_FINGERPRINT, rdata + offset, &len);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    offset += len;
    sw = openpgp_key_get_fingerprint(TAG_KEY_CA2_FINGERPRINT, rdata + offset, &len);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    offset += len;
    sw = openpgp_key_get_fingerprint(TAG_KEY_CA3_FINGERPRINT, rdata + offset, &len);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    offset += len;

    rdata[offset++] = TAG_KEY_GENERATION_DATES;
    rdata[offset++] = KEY_DATETIME_LENGTH * 3;
    sw = openpgp_key_get_datetime(TAG_KEY_SIG_GENERATION_DATES, rdata + offset, &len);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    offset += len;
    sw = openpgp_key_get_datetime(TAG_KEY_DEC_GENERATION_DATES, rdata + offset, &len);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    offset += len;
    sw = openpgp_key_get_datetime(TAG_KEY_AUT_GENERATION_DATES, rdata + offset, &len);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    offset += len;

    uint8_t status;
    rdata[offset++] = TAG_KEY_INFO;
    rdata[offset++] = 6;
    sw = openpgp_key_get_status(OPGP_KEY_SIG, &status);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    rdata[offset++] = 0x01;
    rdata[offset++] = status;
    sw = openpgp_key_get_status(OPGP_KEY_ENC, &status);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    rdata[offset++] = 0x02;
    rdata[offset++] = status;
    sw = openpgp_key_get_status(OPGP_KEY_AUT, &status);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    rdata[offset++] = 0x03;
    rdata[offset++] = status;

    rdata[length_pos] = offset - length_pos - 1;
    rapdu->len = offset;
    return SW_NO_ERROR;
}

static uint16_t get_cardholder_related_data(response_apdu_t *rapdu)
{
    uint16_t sw;
    uint8_t offset = 0;
    size_t len = 0;
    uint8_t *rdata = rapdu->data;

    rdata[offset++] = TAG_NAME;
    sw = openpgp_attr_get_name(rdata + offset + 1, &len);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    rdata[offset++] = len;
    offset += len;

    rdata[offset++] = HI(TAG_LANG);
    rdata[offset++] = LO(TAG_LANG);
    sw = openpgp_attr_get_lang(rdata + offset + 1, &len);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    rdata[offset++] = len;
    offset += len;

    rdata[offset++] = HI(TAG_SEX);
    rdata[offset++] = LO(TAG_SEX);
    sw = openpgp_attr_get_sex(rdata + offset + 1, &len);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    rdata[offset++] = len;
    offset += len;

    rapdu->len = offset;
    return SW_NO_ERROR;
}

static uint16_t get_login_data(response_apdu_t *rapdu)
{
    size_t len = 0;
    uint16_t sw = openpgp_attr_get_login_data(rapdu->data, &len);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    rapdu->len = len;
    return SW_NO_ERROR;
}

static uint16_t get_url_data(response_apdu_t *rapdu)
{
    size_t len = 0;
    uint16_t sw = openpgp_attr_get_url_data(rapdu->data, &len);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    rapdu->len = len;
    return SW_NO_ERROR;
}

static uint16_t get_security_support_template(response_apdu_t *rapdu)
{
    uint16_t sw;
    uint8_t offset = 0;
    size_t len = 0;
    uint8_t *rdata = rapdu->data;

    rdata[offset++] = TAG_DIGITAL_SIG_COUNTER;
    sw = openpgp_attr_get_digital_sig_counter(rdata + offset + 1, &len);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    rdata[offset++] = len;
    offset += len;

    rapdu->len = offset;
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
            return get_aid(rapdu);
        case TAG_HISTORICAL_BYTES:
            return get_historical(rapdu);
        case TAG_PW_STATUS:
            return get_pw_status(rapdu);
        case TAG_APPLICATION_RELATED_DATA:
            return get_application_related_data(rapdu);
        case TAG_CARDHOLDER_RELATED_DATA:
            return get_cardholder_related_data(rapdu);
        case TAG_LOGIN:
            return get_login_data(rapdu);
        case TAG_URL:
            return get_url_data(rapdu);
        case TAG_SECURITY_SUPPORT_TEMPLATE:
            return get_security_support_template(rapdu);
        default:
            return SW_REFERENCE_DATA_NOT_FOUND;
    }

    // 正常終了
    return SW_NO_ERROR;
}

static uint16_t openpgp_ins_put_data(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    return ccid_openpgp_data_put(capdu, rapdu);
}

static uint16_t openpgp_ins_verify(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    return ccid_openpgp_pin_auth(capdu, rapdu);
}

static uint16_t openpgp_ins_terminate(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    return ccid_openpgp_data_terminate(capdu, rapdu);
}

static uint16_t openpgp_ins_activate(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    return ccid_openpgp_data_activate(capdu, rapdu);
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
        case OPENPGP_INS_PUT_DATA:
            rapdu->sw = openpgp_ins_put_data(capdu, rapdu);
            break;
        case OPENPGP_INS_VERIFY:
            rapdu->sw = openpgp_ins_verify(capdu, rapdu);
            break;
        case OPENPGP_INS_TERMINATE:
            rapdu->sw = openpgp_ins_terminate(capdu, rapdu);
            break;
        case OPENPGP_INS_ACTIVATE:
            rapdu->sw = openpgp_ins_activate(capdu, rapdu);
            break;
        default:
            rapdu->sw = SW_INS_NOT_SUPPORTED;
            break;
    }
}

void ccid_openpgp_stop_applet(void)
{
}
