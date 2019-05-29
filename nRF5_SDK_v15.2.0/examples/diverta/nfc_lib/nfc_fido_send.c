/* 
 * File:   nfc_fido_send.c
 * Author: makmorit
 *
 * Created on 2019/05/29, 11:30
 */
#include <stdio.h>

#include "nfc_common.h"
#include "nfc_service.h"
#include "nfc_fido_send.h"

// for logging informations
#define NRF_LOG_MODULE_NAME nfc_fido_send
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// Capability container
const CAPABILITY_CONTAINER NFC_CC = {
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
uint8_t response_buff[NFC_APDU_BUFF_SIZE];

bool nfc_fido_send_response_ex(uint8_t *data, uint8_t data_size, uint16_t status_word)
{
    // データ長チェック
    if (data_size > NFC_APDU_BUFF_SIZE - 2) {
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

bool nfc_fido_send_response(uint16_t resp)
{
	return nfc_fido_send_response_ex(NULL, 0, resp);
}

void nfc_fido_send_app_selection_response(NFC_APPLETS selected_app)
{
    char *version = "U2F_V2";

    if (selected_app == APP_FIDO) {
        nfc_fido_send_response_ex((uint8_t *)version, strlen(version), SW_SUCCESS);

    } else if (selected_app != APP_NOTHING) {
        nfc_fido_send_response(SW_SUCCESS);

    } else {
        nfc_fido_send_response(SW_FILE_NOT_FOUND);
    }
}

void nfc_fido_send_ndef_cc_sample(void)
{
    nfc_fido_send_response_ex((uint8_t *)&NFC_CC, sizeof(CAPABILITY_CONTAINER), SW_SUCCESS);
}

void nfc_fido_send_ndef_tag_sample(APDU_HEADER *apdu)
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
