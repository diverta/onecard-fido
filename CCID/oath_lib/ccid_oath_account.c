/* 
 * File:   ccid_oath_account.c
 * Author: makmorit
 *
 * Created on 2022/06/21, 7:24
 */
#include <string.h>

#include "ccid_oath_account.h"
#include "ccid_oath_define.h"
#include "ccid_oath_object.h"
#include "ccid_oath_totp.h"
#include "ccid_process.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// for debug hex dump data
#define LOG_HEXDUMP_DEBUG_PUT_DATA      false

#ifdef FIDO_ZEPHYR
fido_log_module_register(ccid_oath_account);
#endif

//
// アカウントデータ格納用
//
static char    m_account_name[MAX_NAME_LEN];
static char    m_secret[MAX_KEY_LEN];
static uint8_t m_property;
static uint8_t m_challange[MAX_CHALLENGE_LEN];

// Flash ROM書込み時に実行した関数の参照を保持
static void *m_flash_func = NULL;

static uint8_t get_oath_type(uint8_t alg_byte)
{
    return (alg_byte & OATH_TYPE_MASK);
}

static uint8_t get_oath_alg(uint8_t alg_byte)
{
    return (alg_byte & OATH_ALG_MASK);
}

uint16_t ccid_oath_account_add(command_apdu_t *capdu, response_apdu_t *rapdu)
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

    } else {
        // 属性を未設定状態にする
        m_property = 0;
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

    } else {
        // カウンターを未設定状態にする
        memset(m_challange, 0, sizeof(m_challange));
    }

    // 全体データ長のチェック
    if (offset > capdu->lc) {
        return SW_WRONG_LENGTH;
    }

#if LOG_HEXDUMP_DEBUG_PUT_DATA
    fido_log_print_hexdump_debug(m_account_name, sizeof(m_account_name), "m_account_name");
    fido_log_print_hexdump_debug(m_secret, sizeof(m_secret), "m_secret");
    fido_log_print_hexdump_debug(&m_property, sizeof(m_property), "m_property");
    fido_log_print_hexdump_debug(m_challange, sizeof(m_challange), "m_challange");
#endif

    // 受領データをFlash ROMに設定
    uint16_t sw = ccid_oath_object_account_set(m_account_name, m_secret, m_property, m_challange);
    if (sw == SW_NO_ERROR) {
        // 正常時は、Flash ROM書込みが完了するまで、レスポンスを抑止
        ccid_process_resume_prepare(capdu, rapdu);
        m_flash_func = ccid_oath_account_add;
    }
    return sw;
}

//
// Flash ROM更新後のコールバック関数
//
void ccid_oath_account_retry(void)
{
    uint16_t sw = SW_NO_ERROR;
    if (m_flash_func == ccid_oath_account_add) {
        // 受領データをFlash ROMに設定
        sw = ccid_oath_object_account_set(m_account_name, m_secret, m_property, m_challange);
    }
    if (sw == SW_NO_ERROR) {
        // 正常時は、Flash ROM書込みが完了するまで、レスポンスを抑止
        fido_log_warning("OATH account registration retry");
    } else {
        // 異常時はエラーレスポンス処理を指示
        fido_log_error("OATH data object registration retry fail");
        ccid_process_resume_response(sw);        
    }
}

void ccid_oath_account_resume(bool success)
{
    if (success) {
        // Flash ROM書込みが成功した場合
        uint16_t sw = SW_NO_ERROR;
        if (m_flash_func == ccid_oath_account_add) {
            // TOTPカウンターを使用し、時刻同期を実行
            sw = ccid_oath_totp_set_timestamp(m_secret, m_challange);
            if (sw == SW_NO_ERROR) {
                // 正常終了
                fido_log_info("OATH account registration success");
            }
        }
        ccid_process_resume_response(sw);

    } else {
        // Flash ROM書込みが失敗した場合はエラーレスポンス処理を指示
        fido_log_error("OATH data object registration fail");
        ccid_process_resume_response(SW_UNABLE_TO_PROCESS);
    }
}
