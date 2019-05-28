/* 
 * File:   nfc_service.c
 * Author: makmorit
 *
 * Created on 2019/05/28, 14:21
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "nfc_t4t_lib.h"
#include "app_error.h"

#include "nfc_service.h"

// for logging informations
#define NRF_LOG_MODULE_NAME nfc_service
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for debug hid report
#define NRF_LOG_HEXDUMP_DEBUG_APDU true

// NFCの接続状態を保持
static bool nfc_field_on;

// データ一時格納領域
uint8_t request_buff[NFC_APDU_BUFF_SIZE];
uint8_t response_buff[NFC_APDU_BUFF_SIZE];

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

// アプリケーション選択用
#define AID_NDEF_TYPE_4             "\xD2\x76\x00\x00\x85\x01\x01"
#define AID_NDEF_MIFARE_TYPE_4      "\xD2\x76\x00\x00\x85\x01\x00"
#define AID_CAPABILITY_CONTAINER    "\xE1\x03"
#define AID_NDEF_TAG                "\xE1\x04"
#define AID_FIDO                    "\xa0\x00\x00\x06\x47\x2f\x00\x01"

typedef enum {
    APP_NOTHING = 0,
    APP_NDEF_TYPE_4,
    APP_MIFARE_TYPE_4,
    APP_CAPABILITY_CONTAINER,
    APP_NDEF_TAG,
	APP_FIDO,
} APPLETS;

// 現在選択されているアプリケーション
static APPLETS selected_app;

static bool nfc_write_response_ex(uint8_t *data, uint8_t data_size, uint16_t status_word)
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

static bool nfc_write_response(uint16_t resp)
{
	return nfc_write_response_ex(NULL, 0, resp);
}

static void select_applet(uint8_t *aid, int aid_size)
{
    if (memcmp(aid, AID_FIDO, aid_size) == 0) {
        selected_app = APP_FIDO;
    } else if (memcmp(aid, AID_NDEF_TYPE_4, aid_size) == 0) {
        selected_app = APP_NDEF_TYPE_4;
    } else if (memcmp(aid, AID_CAPABILITY_CONTAINER, aid_size) == 0) {
        selected_app = APP_CAPABILITY_CONTAINER;
    } else if (memcmp(aid, AID_NDEF_TAG, aid_size) == 0) {
        selected_app = APP_NDEF_TAG;
    } else {
        selected_app = APP_NOTHING;
    }
}

static void process_ndef_tag_sample(APDU_HEADER *apdu)
{
    char sw[2];
    char sample_slen = (char)strlen(DIVERTA_SITE_URI);
    char sample_plen = sample_slen + 1;
    char sample_rlen = sample_slen + 5;

    if (apdu->p1 == 0 && apdu->p2 == 0 && apdu->lc == 2) {
        // NDEFサイズ取得要求の場合は、サイズを戻す
        sw[0] = 0x00;
        sw[1] = sample_rlen;
        nfc_write_response_ex((uint8_t *)sw, sizeof(sw), SW_SUCCESS);

    } else {
        // NDEFレコード取得要求の場合は、URLを戻す
        NRF_LOG_DEBUG("NDEF tag sample send");
        sprintf(NDEF_SAMPLE, "%c%c%c%c%c%s", 
            0xd1, 0x01, sample_plen, 'U',
            0x03, DIVERTA_SITE_URI);
        nfc_write_response_ex((uint8_t *)NDEF_SAMPLE, sample_rlen, SW_SUCCESS);
    }
}

static void nfc_data_received(uint8_t *buf, size_t len)
{
#if NRF_LOG_HEXDUMP_DEBUG_APDU
    NRF_LOG_DEBUG("NFC RX data (%d bytes):", len);
    NRF_LOG_HEXDUMP_DEBUG(buf, len);
#endif

    APDU_HEADER *apdu = (APDU_HEADER *)buf;
    uint8_t     *payload = buf + APDU_HEADER_SIZE;
    size_t       plen = (size_t)apdu->lc;
    char        *version = "U2F_V2";
    
    switch(apdu->ins) {
        case APDU_INS_SELECT:
            // アプリケーションを選択
            select_applet(payload, plen);
            if (selected_app == APP_FIDO) {
                nfc_write_response_ex((uint8_t *)version, strlen(version), SW_SUCCESS);
                NRF_LOG_INFO("FIDO applet selected");
            } else if (selected_app != APP_NOTHING) {
                nfc_write_response(SW_SUCCESS);
                NRF_LOG_DEBUG("applet selected (code=%d)", selected_app);
            } else {
                nfc_write_response(SW_FILE_NOT_FOUND);
                NRF_LOG_DEBUG("no applet selected");
            }
            break;

        case APDU_INS_READ_BINARY:
            switch(selected_app) {
                case APP_CAPABILITY_CONTAINER:
                    NRF_LOG_DEBUG("Capability Container send");
                    nfc_write_response_ex((uint8_t *)&NFC_CC, sizeof(CAPABILITY_CONTAINER), SW_SUCCESS);
                    break;
                case APP_NDEF_TAG:
                    // サンプルのNDEFタグを編集
                    //   http://www.diverta.co.jp/
                    process_ndef_tag_sample(apdu);
                    break;
                default:
                    NRF_LOG_ERROR("No binary applet selected");
                    return;
                break;
            }
            break;

        default:
            NRF_LOG_ERROR("Unknown INS %02x", apdu->ins);
			nfc_write_response(SW_INS_INVALID);
            break;
    }
}

static void nfc_callback(void *context, nfc_t4t_event_t event, const uint8_t *data, size_t data_size, uint32_t flags)
{
    UNUSED_PARAMETER(context);

    switch (event) {
        case NFC_T4T_EVENT_FIELD_ON:
            // NFCセッションが開始された時の処理
            if (nfc_field_on == false) {
                NRF_LOG_INFO("NFC Tag has been selected");
                nfc_field_on = true;
            }
            break;

        case NFC_T4T_EVENT_FIELD_OFF:
            // NFCセッションが切断された時の処理
            if (nfc_field_on == true) {
                NRF_LOG_INFO("NFC field lost");
                nfc_field_on = false;
            }
            break;

        case NFC_T4T_EVENT_DATA_IND:
            // データ受信完了時の処理
            if (flags != NFC_T4T_DI_FLAG_MORE) {
                memcpy(request_buff, data, data_size);
                nfc_data_received(request_buff, data_size);
            }
            break;

        default:
            break;
    }
}

void nfc_service_init(void)
{
    nfc_field_on = false;

    // Set up NFC
    ret_code_t err_code = nfc_t4t_setup(nfc_callback, NULL);
    APP_ERROR_CHECK(err_code);

    // Start sensing NFC field
    err_code = nfc_t4t_emulation_start();
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("Set up NFC and started sensing NFC field");
}

void nfc_service_data_send(uint8_t *data, size_t data_size)
{
#if NRF_LOG_HEXDUMP_DEBUG_APDU
    NRF_LOG_DEBUG("NFC TX data (%d bytes):", data_size);
    NRF_LOG_HEXDUMP_DEBUG(data, data_size);
#endif

    // Send the response PDU over NFC.
    ret_code_t err_code = nfc_t4t_response_pdu_send(data, data_size);
    APP_ERROR_CHECK(err_code);
}
