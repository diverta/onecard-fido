/* 
 * File:   ccid_openpgp_attr.c
 * Author: makmorit
 *
 * Created on 2021/02/09, 16:27
 */
#include <string.h>

#include "ccid_openpgp.h"
#include "ccid_openpgp_attr.h"

#define MAX_PIN_LENGTH              64
#define DIGITAL_SIG_COUNTER_LENGTH  3

#define PW_RETRY_COUNTER_DEFAULT    3

#ifdef OPENPGP_TEST_DATA
static char attr_name[] = "Here is the cardname";
#endif

static uint16_t get_retries(uint8_t *retries_pw1, uint8_t *retries_pw3, uint8_t *retries_rc)
{
    // リトライカウンターをFlash ROMから読出し
    // TODO: 仮の実装です。
    *retries_pw1 = PW_RETRY_COUNTER_DEFAULT;
    *retries_pw3 = PW_RETRY_COUNTER_DEFAULT;
    *retries_rc = 0;

    // 正常終了
    return SW_NO_ERROR;
}

uint16_t openpgp_attr_get_pw_status(uint8_t *buf, size_t *size)
{
    // Default PIN strategy
    //   0x00: verify PIN every time
    uint8_t offset = 0;
    buf[offset++] = 0x00;

    // リトライカウンターをFlash ROMから読出し
    uint8_t retries_pw1, retries_pw3, retries_rc;
    uint16_t sw = get_retries(&retries_pw1, &retries_pw3, &retries_rc);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    buf[offset++] = MAX_PIN_LENGTH;
    buf[offset++] = MAX_PIN_LENGTH;
    buf[offset++] = MAX_PIN_LENGTH;
    buf[offset++] = retries_pw1;
    buf[offset++] = retries_rc;
    buf[offset++] = retries_pw3;
    *size = offset;

    // 正常終了
    return SW_NO_ERROR;
}

uint16_t openpgp_attr_get_login_data(uint8_t *buf, size_t *size)
{
    // TODO: 仮の実装です。
    (void)buf;
    *size = 0;

    // 正常終了
    return SW_NO_ERROR;
}

uint16_t openpgp_attr_get_url_data(uint8_t *buf, size_t *size)
{
    // TODO: 仮の実装です。
    (void)buf;
    *size = 0;

    // 正常終了
    return SW_NO_ERROR;
}

uint16_t openpgp_attr_get_digital_sig_counter(uint8_t *buf, size_t *size)
{
    // TODO: 仮の実装です。
    buf[0] = 0x00;
    buf[1] = 0x00;
    buf[2] = 0x01;
    *size = DIGITAL_SIG_COUNTER_LENGTH;

    // 正常終了
    return SW_NO_ERROR;
}

uint16_t openpgp_attr_get_name(uint8_t *buf, size_t *size)
{
    // TODO: 仮の実装です。
    memcpy(buf, attr_name, strlen(attr_name));
    *size = strlen(attr_name);

    // 正常終了
    return SW_NO_ERROR;
}

uint16_t openpgp_attr_get_lang(uint8_t *buf, size_t *size)
{
    // TODO: 仮の実装です。
    (void)buf;
    *size = 0;

    // 正常終了
    return SW_NO_ERROR;
}

uint16_t openpgp_attr_get_sex(uint8_t *buf, size_t *size)
{
    // TODO: 仮の実装です。
    buf[0] = 0x39;
    *size = 1;

    // 正常終了
    return SW_NO_ERROR;
}
