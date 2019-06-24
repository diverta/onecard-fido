/* 
 * File:   fido_nfc_send.c
 * Author: makmorit
 *
 * Created on 2019/05/29, 11:30
 */
#include <stdio.h>
//
// プラットフォーム非依存コード
//
#include "fido_common.h"
#include "fido_nfc_command.h"
#include "fido_nfc_common.h"
#include "fido_nfc_send.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// Capability container
static const CAPABILITY_CONTAINER NFC_CC = {
    .cclen_hi = 0x00, 
    .cclen_lo = 0x0f,
    .version = 0x20,
    .MLe_hi = 0x00, 
    .MLe_lo = 0x7f,
    .MLc_hi = 0x00, 
    .MLc_lo = 0x7f,
    .tlv = {0x04,0x06,0xe1,0x04,0x00,0x7f,0x00,0x00}
};

// サンプルのNDEFデータ編集用
static char  NDEF_SAMPLE[32];
static char *DIVERTA_SITE_URI = "www.diverta.co.jp/";

// データ一時格納領域
//   レスポンス１回あたりの送信データ長の上限＋ステータスワード（２バイト）
#define RESPONSE_BUFF_SIZE (NFC_RESPONSE_MAX_SIZE + 2)
static uint8_t response_buff[RESPONSE_BUFF_SIZE];

// レスポンスデータに関する情報を保持
static uint8_t *m_response_buffer;
static size_t   m_response_length;
static size_t   m_responsed_size;

static bool nfc_fido_send_response_ex(uint8_t *data, uint8_t data_size, uint16_t status_word)
{
    // データ長チェック
    if (data_size > NFC_RESPONSE_MAX_SIZE) {
        return false;
    }
    
    // データを一時領域にコピー
    if (data != NULL && data_size > 0) {
        memcpy(response_buff, data, data_size);
    }

    // NFCステータスワードを末尾に追加
	response_buff[data_size + 0] = status_word >> 8;
	response_buff[data_size + 1] = status_word & 0xff;

    // データ本体とともに送信
	nfc_service_data_send(response_buff, data_size + 2);
	return true;
}

bool fido_nfc_send_response(uint16_t resp)
{
	return nfc_fido_send_response_ex(NULL, 0, resp);
}

void fido_nfc_send_app_selection_response(NFC_APPLETS selected_app)
{
    char *version = U2F_V2_VERSION_STRING;

    if (selected_app == APP_FIDO) {
        nfc_fido_send_response_ex((uint8_t *)version, strlen(version), SW_SUCCESS);

    } else if (selected_app != APP_NOTHING) {
        fido_nfc_send_response(SW_SUCCESS);

    } else {
        fido_nfc_send_response(SW_FILE_NOT_FOUND);
    }
}

void fido_nfc_send_ndef_cc_sample(void)
{
    nfc_fido_send_response_ex((uint8_t *)&NFC_CC, sizeof(CAPABILITY_CONTAINER), SW_SUCCESS);
}

void fido_nfc_send_ndef_tag_sample(APDU_HEADER *apdu)
{
    char sw[2];
    char sample_slen = (char)strlen(DIVERTA_SITE_URI);
    char sample_plen = sample_slen + 1;
    char sample_rlen = sample_slen + 5;

    if (apdu->p1 == 0 && apdu->p2 == 0 && apdu->lc == 2) {
        // NDEFサイズ取得要求の場合は、サイズを戻す
        sw[0] = 0x00;
        sw[1] = sample_rlen;
        nfc_fido_send_response_ex((uint8_t *)sw, sizeof(sw), SW_SUCCESS);

    } else {
        // NDEFレコード取得要求の場合は、URLを戻す
        sprintf(NDEF_SAMPLE, "%c%c%c%c%c%s", 
            0xd1, 0x01, sample_plen, 'U',
            0x03, DIVERTA_SITE_URI);
        nfc_fido_send_response_ex((uint8_t *)NDEF_SAMPLE, sample_rlen, SW_SUCCESS);
    }
}

void fido_nfc_send_command_response_cont(uint8_t get_response_size)
{
    uint16_t status_word;

    // GetResponseコマンド（INS = 0xc0）を受信したら、
    // 次のフレームをレスポンス
    size_t remaining = m_response_length - m_responsed_size;
    if (remaining == 0) {
        return;
    }

    // 今回送信フレーム長を計算
    size_t response_max_size = (get_response_size == 0x00) ? NFC_RESPONSE_MAX_SIZE : get_response_size;
    size_t response_size = (remaining < response_max_size) ? remaining : response_max_size;

    // 次回送信フレーム長を計算
    remaining -= response_size;

    // 次回送信フレーム長に応じ、ステータスワードを生成
    if (remaining == 0) {
        status_word = SW_SUCCESS;
    } else if (remaining < NFC_RESPONSE_MAX_SIZE) {
        status_word = SW_GET_RESPONSE | (0x00ff & (uint16_t)remaining);
    } else {
        status_word = SW_GET_RESPONSE;
    }

    // フレーム送信
    fido_log_debug("APDU sent a frame (%d bytes) status=0x%04x", response_size, status_word);
    uint8_t *response_buffer = m_response_buffer + m_responsed_size;
    nfc_fido_send_response_ex(response_buffer, response_size, status_word);
    m_responsed_size += response_size;

    if (remaining == 0) {
        fido_log_debug("APDU sent completed (%d bytes)", m_responsed_size);
        fido_nfc_command_on_send_completed();
    }
}

void fido_nfc_send_command_response(uint8_t *response_buffer, size_t response_length)
{
    // 引数で指定されたデータを、
    // ISO 7816-4 APDU chainingを使用して
    // 複数フレームに分割してレスポンス
    m_response_buffer = response_buffer;
    m_response_length = response_length;
    m_responsed_size = 0;
    
    if (response_length < NFC_RESPONSE_MAX_SIZE) {
        // 上限バイト以内であれば単一フレームで送信
        fido_nfc_send_command_response_cont((uint8_t)response_length);

    } else {
        // 254バイトを超える場合は残りのフレームは
        // 次回のGetResponseコマンド受信時に送信
        fido_nfc_send_command_response_cont(0x00);
    }
}
