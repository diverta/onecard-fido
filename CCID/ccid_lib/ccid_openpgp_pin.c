/* 
 * File:   ccid_openpgp_pin.c
 * Author: makmorit
 *
 * Created on 2021/02/10, 17:17
 */
#include "ccid_openpgp.h"
#include "ccid_openpgp_pin.h"

static OPGP_PIN_TYPE m_pin_type;
static uint8_t m_pw1_mode;
static bool pw_is_validated;

void ccid_openpgp_pin_type_set(OPGP_PIN_TYPE t)
{
    m_pin_type = t;
}

OPGP_PIN_TYPE ccid_openpgp_pin_type_get(void)
{
    return m_pin_type;
}

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

bool ccid_openpgp_pin_is_validated(void)
{
    return pw_is_validated;
}

void ccid_openpgp_pin_set_validated(bool b)
{
    pw_is_validated = b;
}

uint16_t ccid_openpgp_pin_get_retries(uint8_t *retries)
{
    // TODO: 仮の実装です。
    *retries = 3;
    return SW_NO_ERROR;
}

uint16_t ccid_openpgp_pin_auth_verify(uint8_t *pin_data, size_t pin_size, uint8_t *retries)
{
    // TODO: 仮の実装です。
    // 戻り
    //  SW_UNABLE_TO_PROCESS: IO失敗
    //  SW_WRONG_LENGTH: PINの長さが異なる
    //  SW_AUTHENTICATION_BLOCKED: リトライカウンターが 0
    //  SW_SECURITY_STATUS_NOT_SATISFIED: PINが異なる
    *retries = 3;
    return SW_NO_ERROR;
}
