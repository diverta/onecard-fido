/* 
 * File:   nfc_fido_receive.c
 * Author: makmorit
 *
 * Created on 2019/05/29, 11:03
 */
#include "nfc_common.h"
#include "nfc_fido_send.h"
#include "fido_common.h"

// for logging informations
#define NRF_LOG_MODULE_NAME nfc_fido_receive
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// 現在選択されているアプリケーション
static NFC_APPLETS selected_app;

// FIDOリクエストAPDUを保持
FIDO_APDU_T fido_apdu;

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

void perform_fido_ctap2_message(uint8_t *data, size_t data_size)
{
    APDU_HEADER *apdu = (APDU_HEADER *)data;
    
    if (selected_app != APP_FIDO) {
        nfc_fido_send_response(SW_INS_INVALID);
        return;
    }
    
    // ヘッダー項目をコピー
    fido_apdu.CLA = apdu->cla;
    fido_apdu.INS = apdu->ins;
    fido_apdu.P1 = apdu->p1;
    fido_apdu.P2 = apdu->p2;

    // APDUエンコーディングの判定を行う。
    if (apdu->lc == 0x00) {
        // Extended APDUと判断
        fido_apdu.data = data + APDU_HEADER_SIZE + 2;
        fido_apdu.Lc = (size_t)(
            (data[APDU_HEADER_SIZE + 0] << 8 & 0xFF00) +
             data[APDU_HEADER_SIZE + 1]
            );
        // データ長を計算
        //   実際の受信データ長 - ヘッダー長(7バイト) - フッター長(2バイト)
        fido_apdu.data_length = data_size - APDU_HEADER_SIZE - 4;
        
    } else {
        // Short APDUと判断
        fido_apdu.data = data + APDU_HEADER_SIZE;
        fido_apdu.Lc = apdu->lc;
        // データ長を計算
        //   実際の受信データ長 - ヘッダー長(5バイト) - フッター長(1バイト)
        fido_apdu.data_length = data_size - APDU_HEADER_SIZE - 1;
    }

    NRF_LOG_DEBUG("APDU: data size(%d), Lc(%d bytes)", fido_apdu.data_length, fido_apdu.Lc);
    if (fido_apdu.data_length != fido_apdu.Lc) {
        // データ長が Lcと異なる場合はエラー
        nfc_fido_send_response(SW_WRONG_LENGTH);
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

        case APDU_FIDO_NFCCTAP_MSG:
            // FIDO 2.0機能へ振り分け
            perform_fido_ctap2_message(buf, len);
            break;
            
        default:
            NRF_LOG_ERROR("Unknown INS %02x", apdu->ins);
			nfc_fido_send_response(SW_INS_INVALID);
            break;
    }
}
