/* 
 * File:   ccid_openpgp_crypto.c
 * Author: makmorit
 *
 * Created on 2021/02/23, 12:08
 */
#include "ccid_openpgp.h"
#include "ccid_openpgp_crypto.h"
#include "ccid_openpgp_key.h"
#include "ccid_openpgp_key_rsa.h"
#include "ccid_openpgp_object.h"
#include "ccid_openpgp_pin.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// 鍵属性取得用領域
static uint8_t m_key_attr[16];

static uint16_t get_pw1_status(uint8_t *status)
{
    // PW1ステータスを読み込み
    uint8_t *pw1_status;
    if (ccid_openpgp_object_data_get(TAG_PW_STATUS, &pw1_status, NULL) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // 引数の領域に設定
    if (status != NULL) {
        *status = pw1_status[0];
    }
    return SW_NO_ERROR;
}

static uint16_t compute_digital_signature(command_apdu_t *capdu, response_apdu_t *rapdu)
{
    if (ccid_openpgp_pin_pw1_mode81_get() == 0) {
        return (SW_SECURITY_STATUS_NOT_SATISFIED);
    }

    // PW1ステータスを読み込み
    uint8_t pw1_status;
    uint16_t sw = get_pw1_status(&pw1_status);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    if (pw1_status == 0x00) {
        ccid_openpgp_pin_pw1_mode81_set(false);
    }

    // 鍵ステータスを参照し、鍵がない場合はエラー
    sw = ccid_openpgp_key_is_present(TAG_KEY_SIG);
    if (sw != SW_NO_ERROR) {
        return sw;
    }

    // 鍵属性を取得
    size_t key_attr_size;
    sw = openpgp_key_get_attributes(TAG_ALGORITHM_ATTRIBUTES_SIG, m_key_attr, &key_attr_size);
    if (sw != SW_NO_ERROR) {
        return sw;
    }

    // RSA秘密鍵による署名を生成
    size_t signature_size;
    sw = ccid_openpgp_key_rsa_signature(TAG_KEY_SIG, m_key_attr, capdu->data, capdu->lc, rapdu->data, &signature_size);
    if (sw != SW_NO_ERROR) {
        return sw;
    }

    // 正常終了
    rapdu->len = signature_size;
    return SW_NO_ERROR;
}

uint16_t ccid_openpgp_crypto_pso(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    if (capdu->p1 == 0x9e && capdu->p2 == 0x9a) {
        // RSA鍵を使用し署名生成
        return compute_digital_signature(capdu, rapdu);
    }

    return SW_WRONG_P1P2;
}
