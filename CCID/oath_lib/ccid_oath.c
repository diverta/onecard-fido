/* 
 * File:   ccid_oath.c
 * Author: makmorit
 *
 * Created on 2022/04/29, 10:42
 */
#include <string.h>

#include "ccid_oath.h"
#include "ccid_oath_define.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// for debug hex dump data
#define LOG_HEXDUMP_DEBUG_PUT_DATA      false

#ifdef FIDO_ZEPHYR
fido_log_module_register(ccid_oath);
#endif

//
// アカウントデータ格納用
//
static char    m_account_name[MAX_NAME_LEN];
static char    m_secret[MAX_KEY_LEN];
static uint8_t m_property;
static uint8_t m_challange[MAX_CHALLENGE_LEN];

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

static uint8_t get_oath_type(uint8_t alg_byte)
{
    return (alg_byte & OATH_TYPE_MASK);
}

static uint8_t get_oath_alg(uint8_t alg_byte)
{
    return (alg_byte & OATH_ALG_MASK);
}

static uint16_t oath_ins_put(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // パラメーターのチェック
    if (capdu->p1 != 0x00 || capdu->p2 != 0x00) {
        return SW_WRONG_P1P2;
    }

    //
    // アカウント名を抽出
    //
    uint8_t offset = 0;
    if (offset + 1 >= capdu->lc) {
        return SW_WRONG_LENGTH;
    }
    if (capdu->data[offset++] != OATH_TAG_NAME) {
        return SW_WRONG_DATA;
    }
    uint8_t name_len = capdu->data[offset++];
    if (name_len > MAX_NAME_LEN || name_len == 0) {
        return SW_WRONG_DATA;
    }
    uint8_t name_offset = offset;
    if (name_offset >= capdu->lc) {
        return SW_WRONG_LENGTH;
    }

    // アカウント名を保持
    memcpy(m_account_name, capdu->data + name_offset, name_len);
    offset += name_len;

    //
    // Secretを抽出
    //
    if (capdu->data[offset] == OATH_TAG_KEY) {
        offset++;

        // データ長のチェック
        if (offset >= capdu->lc) {
            return SW_WRONG_LENGTH;
        }
        uint8_t key_len = capdu->data[offset++];
        if (key_len > MAX_KEY_LEN || key_len <= 2) {
            // 2 for algo & digits
            return SW_WRONG_DATA;
        }
        uint8_t key_offset = offset;
        if (key_offset >= capdu->lc) {
            return SW_WRONG_LENGTH;
        }

        // 種別・アルゴリズムのチェック
        uint8_t alg = capdu->data[offset];
        uint8_t oath_type = get_oath_type(alg);
        uint8_t oath_alg = get_oath_alg(alg);
        if ((oath_type != OATH_TYPE_HOTP && oath_type != OATH_TYPE_TOTP) ||
            (oath_alg  != OATH_ALG_SHA1  && oath_alg  != OATH_ALG_SHA256)){
            return SW_WRONG_DATA;
        }
        if (offset + 1 >= capdu->lc) {
            return SW_WRONG_LENGTH ;
        }
        uint8_t digits = capdu->data[offset + 1];
        if (digits < 4 || digits > 8) {
            return SW_WRONG_DATA;
        }

        // Secretを保持
        memcpy(m_secret, capdu->data + key_offset, key_len);
        offset += key_len;
    }

    //
    // オプション属性を抽出
    //
    if (capdu->data[offset] == OATH_TAG_PROPERTY) {
        offset++;

        // データ長のチェック
        if (offset >= capdu->lc) {
            return SW_WRONG_LENGTH;
        }
        uint8_t prop_len = capdu->data[offset++];
        if (prop_len != 1) {
            return SW_WRONG_DATA;
        }
        uint8_t prop_offset = offset;
        if (prop_offset >= capdu->lc) {
            return SW_WRONG_LENGTH;
        }

        // データのチェック
        uint8_t prop = capdu->data[prop_offset];
        if ((prop & ~OATH_PROP_ALL_FLAGS) != 0) {
            return SW_WRONG_DATA;
        }

        // オプション属性を保持
        m_property = prop;
        offset++;
    }

    //
    // カウンターを抽出
    //
    if (capdu->data[offset] == OATH_TAG_COUNTER) {
        offset++;

        // データ長のチェック
        if (offset >= capdu->lc) {
            return SW_WRONG_LENGTH;
        }
        uint8_t counter_len = capdu->data[offset++];
        if (counter_len != 4) {
            return SW_WRONG_DATA;
        }
        uint8_t counter_offset = offset;
        if (counter_offset >= capdu->lc) {
            return SW_WRONG_LENGTH;
        }

        // カウンターを保持
        memcpy(m_challange + 4, capdu->data + counter_offset, counter_len);
        offset += counter_len;
    }

    // 全体データ長のチェック
    if (offset > capdu->lc) {
        return SW_WRONG_LENGTH;
    }

    // TODO: 受領データを永続化
#if LOG_HEXDUMP_DEBUG_PUT_DATA
    fido_log_print_hexdump_debug(m_account_name, sizeof(m_account_name), "m_account_name");
    fido_log_print_hexdump_debug(m_secret, sizeof(m_secret), "m_secret");
    fido_log_print_hexdump_debug(&m_property, sizeof(m_property), "m_property");
    fido_log_print_hexdump_debug(m_challange, sizeof(m_challange), "m_challange");
#endif

    return SW_NO_ERROR;
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
        default:
            rapdu->sw = SW_INS_NOT_SUPPORTED;
            break;
    }
}

void ccid_oath_stop_applet(void)
{
    fido_log_debug("Applet OATH stopped");
}
