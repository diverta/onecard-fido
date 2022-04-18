/* 
 * File:   ccid_openpgp_attr.c
 * Author: makmorit
 *
 * Created on 2021/02/09, 16:27
 */
#include <string.h>

#include "ccid_openpgp.h"
#include "ccid_openpgp_attr.h"
#include "ccid_openpgp_object.h"
#include "ccid_pin_auth.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(ccid_openpgp_attr);
#endif

#define MAX_PIN_LENGTH              64
#define DIGITAL_SIG_COUNTER_LENGTH  3

#define PW_RETRY_COUNTER_DEFAULT    3

#ifdef OPENPGP_TEST_DATA
static char attr_name[] = "Here is the cardname";
#endif

// 一時格納領域
static uint8_t m_work_buf[8];

uint16_t ccid_openpgp_attr_get_retries(PIN_TYPE type, uint8_t *retries)
{
    // リトライカウンターをFlash ROMから読出し
    PIN_T *pw = ccid_pin_auth_pin_t(type);
    uint16_t sw = ccid_pin_auth_get_retries(pw);
    if (sw != SW_NO_ERROR) {
        return sw;
    }

    // 正常終了
    *retries = pw->current_retries;
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
    uint16_t sw;
    // PW1
    sw = ccid_openpgp_attr_get_retries(OPGP_PIN_PW1, &retries_pw1);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    // PW3
    sw = ccid_openpgp_attr_get_retries(OPGP_PIN_PW3, &retries_pw3);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    // RC
    sw = ccid_openpgp_attr_get_retries(OPGP_PIN_RC, &retries_rc);
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
    bool is_exist = false;
    size_t buffer_size;
    if (ccid_flash_openpgp_object_read(APPLET_OPENPGP, TAG_DIGITAL_SIG_COUNTER, &is_exist, buf, &buffer_size) == false) {
        // 読出しが失敗した場合はエラー
        fido_log_error("OpenPGP sign counter read fail: tag=0x%04x", TAG_DIGITAL_SIG_COUNTER);
        return SW_UNABLE_TO_PROCESS;
    }
    if (is_exist == false) {
        // Flash ROMに登録されていない場合はデフォルトを設定
        buffer_size = TAG_DIGITAL_SIG_COUNTER;
        memset(buf, 0, TAG_DIGITAL_SIG_COUNTER);
    }

    // サイズを戻す
    if (size != NULL) {
        *size = buffer_size;
    }

    // 正常終了
    return SW_NO_ERROR;
}

uint16_t openpgp_attr_set_digital_sig_counter(uint32_t counter)
{
    // カウンター値をビッグエンディアンで設定
    m_work_buf[0] = counter >> 16 & 0xff;
    m_work_buf[1] = counter >>  8 & 0xff;
    m_work_buf[2] = counter >>  0 & 0xff;

    // タイムスタンプをFlash ROMに設定
    if (ccid_openpgp_object_data_set(TAG_DIGITAL_SIG_COUNTER, m_work_buf, DIGITAL_SIG_COUNTER_LENGTH) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

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
