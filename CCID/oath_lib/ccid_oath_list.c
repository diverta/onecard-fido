/* 
 * File:   ccid_oath_list.c
 * Author: makmorit
 *
 * Created on 2022/08/25, 16:12
 */
#include "ccid_define.h"
#include "ccid_oath.h"
#include "ccid_oath_define.h"
#include "ccid_oath_list.h"
#include "ccid_oath_object.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(ccid_oath_list);
#endif

//
// fetch_account_data関数内で共有する変数
//
static command_apdu_t  *m_capdu;
static response_apdu_t *m_rapdu;
static size_t           m_offset;

static int fetch_account_data(uint8_t *data, size_t size)
{
    //
    // data の内容
    //   0   : アカウント（64バイト）
    //   64  : アカウント長（1バイト）
    //   65  : アルゴリズム（1バイト）
    //   66  : OTP桁数（1バイト）
    //
    // アカウント長
    size_t name_size = data[64];

    // レスポンスデータを生成
    m_rapdu->data[m_offset++] = OATH_TAG_NAME;
    m_rapdu->data[m_offset++] = name_size;
    memcpy(m_rapdu->data + m_offset, data, name_size);
    m_offset += name_size;
    m_rapdu->data[m_offset++] = OATH_TAG_META;
    m_rapdu->data[m_offset++] = 2;
    m_rapdu->data[m_offset++] = data[65];
    m_rapdu->data[m_offset++] = data[66];
    return 0;
}

uint16_t ccid_oath_list(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // パラメーターのチェック
    if (capdu->p1 != 0x00 || capdu->p2 != 0x00) {
        return SW_WRONG_P1P2;
    }

    // 外部変数に設定
    m_capdu = capdu;
    m_rapdu = rapdu;
    m_offset = 0;

    // アカウントデータを読み込み
    if (ccid_oath_object_account_fetch(fetch_account_data) != SW_NO_ERROR) {
        return SW_UNABLE_TO_PROCESS;
    }

    // レスポンス長を設定
    rapdu->len = m_offset;
    return SW_NO_ERROR;
}
