/* 
 * File:   nfc_fido_receive.c
 * Author: makmorit
 *
 * Created on 2019/05/29, 11:03
 */
#include "nfc_common.h"
#include "nfc_fido_send.h"

// for logging informations
#define NRF_LOG_MODULE_NAME nfc_fido_receive
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// 現在選択されているアプリケーション
static NFC_APPLETS selected_app;

static void select_applet(uint8_t *aid, int aid_size)
{
    if (memcmp(aid, AID_FIDO, aid_size) == 0) {
        selected_app = APP_FIDO;
        NRF_LOG_DEBUG("FIDO application select");

    } else if (memcmp(aid, AID_NDEF_TYPE_4, aid_size) == 0) {
        selected_app = APP_NDEF_TYPE_4;
        NRF_LOG_DEBUG("NDEF tag type-4 application select");

    } else if (memcmp(aid, AID_CAPABILITY_CONTAINER, aid_size) == 0) {
        selected_app = APP_CAPABILITY_CONTAINER;
        NRF_LOG_DEBUG("Capability Container select");

    } else if (memcmp(aid, AID_NDEF_TAG, aid_size) == 0) {
        selected_app = APP_NDEF_TAG;
        NRF_LOG_DEBUG("NDEF select");

    } else {
        selected_app = APP_NOTHING;
        NRF_LOG_ERROR("NO applet selected");
    }
}

static void perform_read_binary(APDU_HEADER *apdu)
{
    switch(selected_app) {
        case APP_CAPABILITY_CONTAINER:
            nfc_fido_send_ndef_cc_sample();
            break;
        case APP_NDEF_TAG:
            // サンプルのNDEFタグを編集
            //   http://www.diverta.co.jp/
            nfc_fido_send_ndef_tag_sample(apdu);
            break;
        default:
            NRF_LOG_ERROR("No binary applet selected");
            return;
        break;
    }
}

void nfc_fido_receive_request_frame(uint8_t *buf, size_t len)
{
    APDU_HEADER *apdu = (APDU_HEADER *)buf;
    uint8_t     *payload = buf + APDU_HEADER_SIZE;
    size_t       plen = (size_t)apdu->lc;
    
    switch(apdu->ins) {
        case APDU_INS_SELECT:
            // アプリケーションを選択し、レスポンスを送信
            select_applet(payload, plen);
            nfc_fido_send_app_selection_response(selected_app);
            break;

        case APDU_INS_READ_BINARY:
            // NDEFメッセージをレスポンス
            perform_read_binary(apdu);
            break;

        default:
            NRF_LOG_ERROR("Unknown INS %02x", apdu->ins);
			nfc_fido_send_response(SW_INS_INVALID);
            break;
    }
}
