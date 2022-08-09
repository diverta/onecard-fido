/* 
 * File:   ccid_oath_calculate.c
 * Author: makmorit
 *
 * Created on 2022/07/11, 17:03
 */
#include "ccid_oath.h"
#include "ccid_oath_define.h"
#include "ccid_oath_object.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// for debug
#define LOG_ACCOUNT_EXIST_AND_READ      false

#ifdef FIDO_ZEPHYR
fido_log_module_register(ccid_oath_calculate);
#endif

//
// アカウントデータ格納用
//
static char    m_account_name[MAX_NAME_LEN];
static uint8_t m_secret[MAX_KEY_LEN];
static uint8_t m_property;
static uint8_t m_challenge[MAX_CHALLENGE_LEN];

uint16_t ccid_oath_calculate(command_apdu_t *capdu, response_apdu_t *rapdu) 
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
        return(SW_WRONG_LENGTH);
    }
    if (capdu->data[offset++] != OATH_TAG_NAME) {
        return SW_WRONG_DATA;
    }
    uint8_t name_len = capdu->data[offset++];
    if (name_len > MAX_NAME_LEN || name_len == 0) {
        return SW_WRONG_DATA;
    }
    uint8_t name_offset = offset;
    offset += name_len;
    if (offset > capdu->lc) {
        return SW_WRONG_LENGTH;
    }

    // アカウント名をバッファにコピー
    memset(m_account_name, 0, sizeof(m_account_name));
    memcpy(m_account_name, capdu->data + name_offset, name_len);

    //
    // アカウントデータを取得
    //
    bool exist = false;
    uint8_t secret_size;
    uint16_t sw = ccid_oath_object_account_read(m_account_name, name_len, m_secret, &secret_size, &m_property, m_challenge, &exist);

#if LOG_ACCOUNT_EXIST_AND_READ
    fido_log_debug("account record(%s): exist=%d", log_strdup(m_account_name), exist);
    fido_log_print_hexdump_debug(m_secret, secret_size, "secret");
    fido_log_print_hexdump_debug(m_challenge, MAX_CHALLENGE_LEN, "challenge");
#endif
    if (exist == false) {
        return SW_DATA_INVALID;
    }

    return sw;
}
