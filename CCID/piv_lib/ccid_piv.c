/* 
 * File:   ccid_piv.c
 * Author: makmorit
 *
 * Created on 2020/06/01, 9:55
 */
#include "ccid.h"
#include "ccid_piv.h"
#include "ccid_piv_general_auth.h"
#include "ccid_piv_object.h"
#include "ccid_piv_object_import.h"
#include "ccid_piv_pin.h"
#include "ccid_ykpiv.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(ccid_piv);
#endif

//
// 各種ID
//  NIST RID
//  NIST PIX (1st version of the PIV Card Application)
//
static const uint8_t rid[] = {0xa0, 0x00, 0x00, 0x03, 0x08};
static const uint8_t pix[] = {0x00, 0x00, 0x10, 0x00, 0x01, 0x00};
static const uint8_t rid_size = sizeof(rid);
static const uint8_t pix_size = sizeof(pix);

bool ccid_piv_rid_is_piv_applet(command_apdu_t *capdu)
{
    return (capdu->lc >= rid_size &&
            memcmp(capdu->data, rid, rid_size) == 0);
}

//
// 管理コマンドが実行可能かどうかを保持
//
static bool admin_mode;

bool ccid_piv_admin_mode_get(void)
{
    return admin_mode;
}

void ccid_piv_admin_mode_set(bool mode)
{
    admin_mode = mode;
}

static uint16_t piv_ins_select(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // パラメーターのチェック
    if (capdu->p1 != 0x04 || capdu->p2 != 0x00) {
        return SW_WRONG_P1P2;
    }

    // レスポンスデータを編集
    uint8_t *rdata = rapdu->data;
    rdata[0] = 0x61;
    rdata[1] = 6 + pix_size + rid_size;
    rdata[2] = 0x4f;
    rdata[3] = pix_size;
    memcpy(rdata + 4, pix, pix_size);
    rdata[4 + pix_size] = 0x79;
    rdata[5 + pix_size] = 2 + rid_size;
    rdata[6 + pix_size] = 0x4F;
    rdata[7 + pix_size] = rid_size;
    memcpy(rdata + 8 + pix_size, rid, rid_size);
    rapdu->len = 8 + pix_size + rid_size;

    // 正常終了
    return SW_NO_ERROR;
}

static uint16_t piv_ins_get_data(command_apdu_t *capdu, response_apdu_t *rapdu)
{
    // 送受信APDUデータの格納領域
    uint8_t *data = capdu->data;
    uint8_t *rdata = rapdu->data;

    // パラメーターのチェック
    if (capdu->p1 != 0x3f || capdu->p2 != 0xff) {
        return SW_WRONG_P1P2;
    }
    if (data[0] != 0x5c) {
        return SW_WRONG_DATA;
    }
    if (data[1] + 2 != capdu->lc) {
        return SW_WRONG_LENGTH;
    }

    if (data[1] == 1) {
        if (data[2] != 0x7e) {
            return SW_FILE_NOT_FOUND;
        }
        // For the Discovery Object, the 0x7e template nests two data elements:
        // 1) tag 0x4f contains the AID of the PIV Card Application and
        // 2) tag 0x5f2f lists the PIN Usage Policy.
        uint8_t aid_size = rid_size + pix_size;
        rdata[0] = 0x7e;
        rdata[1] = 5 + aid_size + ccid_piv_pin_policy_size();
        rdata[2] = 0x4f;
        rdata[3] = aid_size;
        memcpy(rdata + 4, rid, rid_size);
        memcpy(rdata + 4 + rid_size, pix, pix_size);
        rdata[4 + aid_size] = 0x5f;
        rdata[5 + aid_size] = 0x2f;
        rdata[6 + aid_size] = ccid_piv_pin_policy_size();
        memcpy(rdata + 7 + aid_size, ccid_piv_pin_policy(), ccid_piv_pin_policy_size());
        rapdu->len = 7 + aid_size + ccid_piv_pin_policy_size();

        fido_log_debug("Discovery Object is requested (%d bytes)", rapdu->len);

    } else if (data[1] == 3) {
        if (capdu->lc != 5 || data[2] != 0x5f || data[3] != 0xc1) {
            return SW_FILE_NOT_FOUND;
        }
        // ファイル種別を取得し、
        // ファイルの内容を送信APDUデータに格納
        size_t size = ccid_response_apdu_size_max();
        if (ccid_piv_object_get(data[4], rdata, &size) == false) {
            return SW_FILE_NOT_FOUND;
        }
        rapdu->len = (uint16_t)size;

    } else {
        return SW_FILE_NOT_FOUND;
    }

    // 正常終了
    return SW_NO_ERROR;
}

static uint16_t piv_ins_general_authenticate(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    return ccid_piv_general_authenticate(capdu, rapdu);
}

static uint16_t piv_ins_put_data(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    return ccid_piv_object_import(capdu, rapdu);
}

static uint16_t piv_ins_verify(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    return ccid_piv_pin_auth(capdu, rapdu);
}

static uint16_t piv_ins_change_reference_data(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    return ccid_piv_pin_set(capdu, rapdu);
}

static uint16_t piv_ins_reset_retry_counter(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    return ccid_piv_pin_reset(capdu, rapdu);
}

static void piv_init(void)
{
    // 初期化処理を一度だけ実行
    static bool initialized = false;
    if (initialized) {
        return;
    }

    // PIN／リトライカウンターの初期化
    if (ccid_piv_pin_init() == false) {
        return;
    }

    // 初期化処理完了
    initialized = true;
}

void ccid_piv_apdu_process(command_apdu_t *capdu, response_apdu_t *rapdu)
{
    // 初期化処理を一度だけ実行
    piv_init();

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
        case PIV_INS_GET_DATA:
            rapdu->sw = piv_ins_get_data(capdu, rapdu);
            break;
        case PIV_INS_GENERAL_AUTHENTICATE:
            rapdu->sw = piv_ins_general_authenticate(capdu, rapdu);
            break;
        case PIV_INS_PUT_DATA:
            rapdu->sw = piv_ins_put_data(capdu, rapdu);
            break;
        case PIV_INS_VERIFY:
            rapdu->sw = piv_ins_verify(capdu, rapdu);
            break;
        case PIV_INS_CHANGE_REFERENCE_DATA:
            rapdu->sw = piv_ins_change_reference_data(capdu, rapdu);
            break;
        case PIV_INS_RESET_RETRY_COUNTER:
            rapdu->sw = piv_ins_reset_retry_counter(capdu, rapdu);
            break;
        //
        // Yubico PIV Tool固有のコマンド
        //
        case YKPIV_INS_GET_VERSION:
            rapdu->sw = ccid_ykpiv_ins_get_version(capdu, rapdu);
            break;
        case YKPIV_INS_GET_SERIAL:
            rapdu->sw = ccid_ykpiv_ins_get_serial(capdu, rapdu);
            break;
        case YKPIV_INS_SET_MGMKEY:
            rapdu->sw = ccid_ykpiv_ins_set_mgmkey(capdu, rapdu);
            break;
        case YKPIV_INS_IMPORT_ASYMMETRIC_KEY:
            rapdu->sw = ccid_ykpiv_ins_import_key(capdu, rapdu);
            break;
        case YKPIV_INS_RESET:
            rapdu->sw = ccid_ykpiv_ins_reset(capdu, rapdu);
            break;
        default:
            rapdu->sw = SW_INS_NOT_SUPPORTED;
            break;
    }
}

void ccid_piv_stop_applet(void)
{
    ccid_piv_admin_mode_set(false);
}
