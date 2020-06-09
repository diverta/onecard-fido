/* 
 * File:   ccid_piv.c
 * Author: makmorit
 *
 * Created on 2020/06/01, 9:55
 */
#include <stdlib.h>

#include "ccid.h"
#include "ccid_piv.h"
#include "ccid_piv_general_auth.h"
#include "ccid_piv_object.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

static const uint8_t rid[] = {0xa0, 0x00, 0x00, 0x03, 0x08};
static const uint8_t pix[] = {0x00, 0x00, 0x10, 0x00, 0x01, 0x00};
static const uint8_t pin_policy[] = {0x40, 0x10};

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

bool ccid_piv_object_get(uint8_t file_tag, uint8_t *buffer, size_t *size)
{
    // パラメーターチェック
    if (*size < 1) {
        return false;
    }
    
    // PIVデータのタグごとに処理を分岐
    bool success = false;
    switch (file_tag) {
        case 0x01:
            // X.509 Certificate for Card Authentication
            success = ccid_piv_object_cert_cauth_get(buffer, size);
            break;
        case 0x02:
            // Card Holder Unique Identifier
            success = ccid_piv_object_chuid_get(buffer, size);
            break;
        case 0x05:
            // X.509 Certificate for PIV Authentication
            success = ccid_piv_object_cert_pauth_get(buffer, size);
            break;
        case 0x07:
            // Card Capability Container
            success = ccid_piv_object_ccc_get(buffer, size);
            break;
        case 0x0A:
            // X.509 Certificate for Digital Signature
            success = ccid_piv_object_cert_digsig_get(buffer, size);
            break;
        case 0x0B:
            // X.509 Certificate for Key Management
            success = ccid_piv_object_cert_keyman_get(buffer, size);
            break;
        case 0x0C:
            // Key History Object
            success = ccid_piv_object_key_history_get(buffer, size);
            break;
        default:
            break;
    }
    
    if (success == false) {
        // 処理失敗時は長さをゼロクリア
        *size = 0;
    }
    
    // 正常終了
    return success;
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
        rdata[0] = 0x7e;
        rdata[1] = 5 + sizeof(rid) + sizeof(pix) + sizeof(pin_policy);
        rdata[2] = 0x4f;
        rdata[3] = sizeof(rid) + sizeof(pix);
        memcpy(rdata + 4, rid, sizeof(rid));
        memcpy(rdata + 4 + sizeof(rid), pix, sizeof(pix));
        rdata[4 + sizeof(rid) + sizeof(pix)] = 0x5f;
        rdata[5 + sizeof(rid) + sizeof(pix)] = 0x2f;
        rdata[6 + sizeof(rid) + sizeof(pix)] = sizeof(pin_policy);
        memcpy(rdata + 7 + sizeof(rid) + sizeof(pix), pin_policy, sizeof(pin_policy));
        rapdu->len = 7 + sizeof(rid) + sizeof(pix) + sizeof(pin_policy);

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

static uint16_t piv_ins_get_version(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // パラメーターのチェック
    if (capdu->p1 != 0x00 || capdu->p2 != 0x00) {
        return SW_WRONG_P1P2;
    }
    if (capdu->lc != 0) {
        return SW_WRONG_LENGTH;
    }

    // バージョン文字列 "xx.xx.xx" を分割
    uint8_t v[] = {0x00, 0x00, 0x00};
#ifdef FW_REV
    char *version_str = FW_REV;
    char *tp = strtok(version_str, ".");
    for (int i = 0; tp != NULL; i++) {
        v[i] = atoi(tp);
        tp = strtok(NULL, ".");
    }    
#endif
    
    // レスポンスデータを編集
    uint8_t *rdata = rapdu->data;
    rdata[0] = v[0];
    rdata[1] = v[1];
    rdata[2] = v[2];
    rapdu->len = 3;

    // 正常終了
    return SW_NO_ERROR;
}

static uint16_t piv_ins_get_serial(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // パラメーターのチェック
    if (capdu->p1 != 0x00 || capdu->p2 != 0x00) {
        return SW_WRONG_P1P2;
    }
    if (capdu->lc != 0) {
        return SW_WRONG_LENGTH;
    }

    // ファイルの内容を送信APDUデータに格納
    uint8_t *rdata = rapdu->data;
    size_t size = ccid_response_apdu_size_max();
    if (ccid_piv_object_sn_get(rdata, &size) == false) {
        return SW_FILE_NOT_FOUND;
    }
    rapdu->len = (uint16_t)size;

    // 正常終了
    return SW_NO_ERROR;
}

static uint16_t piv_ins_put_data(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // 管理コマンドが実行可能でない場合は終了
    if (ccid_piv_admin_mode_get() == false) {
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }

    //
    // TODO: ここに処理を記述
    //

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
        case PIV_INS_GET_DATA:
            rapdu->sw = piv_ins_get_data(capdu, rapdu);
            break;
        case PIV_INS_GET_VERSION:
            rapdu->sw = piv_ins_get_version(capdu, rapdu);
            break;
        case PIV_INS_GET_SERIAL:
            rapdu->sw = piv_ins_get_serial(capdu, rapdu);
            break;
        case PIV_INS_GENERAL_AUTHENTICATE:
            rapdu->sw = piv_ins_general_authenticate(capdu, rapdu);
            break;
        case PIV_INS_PUT_DATA:
            rapdu->sw = piv_ins_put_data(capdu, rapdu);
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
