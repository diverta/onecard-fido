/* 
 * File:   ccid_openpgp_pin.c
 * Author: makmorit
 *
 * Created on 2021/02/10, 17:17
 */
#include "ccid_openpgp.h"
#include "ccid_openpgp_pin.h"
#include "ccid_pin_auth.h"

//
// PIN種別／認証モードを保持
//
static PIN_T  *m_pw;
static uint8_t m_pw1_mode;

void ccid_openpgp_pin_pw1_mode81_set(bool b)
{
    if (b) {
        m_pw1_mode |= 0x01;
    } else {
        m_pw1_mode &= 0xfe;
    }
}

void ccid_openpgp_pin_pw1_mode82_set(bool b)
{
    if (b) {
        m_pw1_mode |= 0x02;
    } else {
        m_pw1_mode &= 0xfd;
    }
}

uint16_t ccid_openpgp_pin_auth(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // パラメーターのチェック
    if (capdu->p1 != 0x00 && capdu->p1 != 0xff) {
        return SW_WRONG_P1P2;
    }

    // PIN種別判定／認証済みフラグをクリア
    if (capdu->p2 == 0x81) {
        m_pw = ccid_pin_auth_pin_t(OPGP_PIN_PW1);
        ccid_openpgp_pin_pw1_mode81_set(false);
    } else if (capdu->p2 == 0x82) {
        m_pw = ccid_pin_auth_pin_t(OPGP_PIN_PW1);
        ccid_openpgp_pin_pw1_mode82_set(false);
    } else if (capdu->p2 == 0x83) {
        m_pw = ccid_pin_auth_pin_t(OPGP_PIN_PW3);
    } else {
        return SW_WRONG_P1P2;
    }

    // PIN認証クリアの場合
    if (capdu->p1 == 0xff) {
        m_pw->is_validated = false;
        return SW_NO_ERROR;
    }

    // PINリトライカウンター照会の場合
    if (capdu->lc == 0) {
        if (m_pw->is_validated) {
            return SW_NO_ERROR;
        }
        // Flash ROMに登録されているリトライカウンターを応答
        uint16_t sw = ccid_pin_auth_get_retries(m_pw);
        if (sw != SW_NO_ERROR) {
            return sw;
        } else {
            return SW_PIN_RETRIES + m_pw->current_retries;
        }
    }

    // 入力PINコードで認証
    uint16_t sw = ccid_pin_auth_verify(m_pw, capdu->data, capdu->lc);
    if (sw != SW_NO_ERROR) {
        return sw;
    }

    // 認証済みフラグを設定
    if (capdu->p2 == 0x81) {
        ccid_openpgp_pin_pw1_mode81_set(true);
    } else if (capdu->p2 == 0x82) {
        ccid_openpgp_pin_pw1_mode82_set(true);
    }

    // 正常終了
    return SW_NO_ERROR;    
}
