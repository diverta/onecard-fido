/* 
 * File:   nfc_fido_receive.c
 * Author: makmorit
 *
 * Created on 2019/05/29, 11:03
 */
#include "nfc_common.h"
#include "nfc_fido_send.h"
#include "fido_common.h"
#include "fido_ctap2_command.h"
#include "nfc_fido_command.h"

// for logging informations
#define NRF_LOG_MODULE_NAME nfc_fido_receive
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for debug apdu data
#define NRF_LOG_DEBUG_APDU_FRAME_BUFF false
#define NRF_LOG_DEBUG_APDU_BUFF       false
#define NRF_LOG_DEBUG_BUFF (NRF_LOG_DEBUG_APDU_FRAME_BUFF || NRF_LOG_DEBUG_APDU_BUFF)

// 現在選択されているアプリケーション
static NFC_APPLETS selected_app;

// FIDOリクエストAPDUを保持
static FIDO_APDU_T fido_apdu;

// FIDOリクエストAPDU編集用の作業領域（固定長）
static uint8_t apdu_data_buffer[APDU_DATA_MAX_LENGTH];

#if NRF_LOG_DEBUG_BUFF
static void print_hexdump_debug(uint8_t *buff, size_t size)
{
    int j, k;
    for (j = 0; j < size; j += 64) {
        k = size - j;
        NRF_LOG_HEXDUMP_DEBUG(buff + j, (k < 64) ? k : 64);
    }
}
#endif

FIDO_APDU_T *nfc_fido_receive_apdu(void)
{
    return &fido_apdu;
}

static void clear_work_area()
{
    // APDU領域を初期化
    memset(&fido_apdu, 0, sizeof(FIDO_APDU_T));
    memset(apdu_data_buffer, 0, APDU_DATA_MAX_LENGTH);
    fido_apdu.data = apdu_data_buffer;
}

static void select_applet(uint8_t *aid, int aid_size)
{
    // モジュール内の一時変数を初期化
    clear_work_area();

    // アプリケーションIDごとに処理を分岐
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

static bool is_last_nfc_frame(APDU_HEADER *apdu)
{
    // 0x80=true, 0x90=false
    return !(apdu->cla & 0x10);
}

static uint16_t process_with_nfc_header(uint8_t *data, size_t data_size, FIDO_APDU_T *fido_apdu)
{
    APDU_HEADER *apdu = (APDU_HEADER *)data;
    uint8_t     *data_in_frame;
    size_t       data_in_frame_size;
    size_t       real_size;
    
    // APDU内のヘッダー部／フッター部の長さ
    uint8_t apdu_header_size;
    uint8_t apdu_footer_size;

    // APDUエンコーディングの判定を行う。
    if (apdu->lc == 0x00) {
        // Extended APDUと判断し、Lc、Leバイトの長さを設定
        apdu_header_size = APDU_HEADER_SIZE + 2;
        if (is_last_nfc_frame(apdu)) {
            apdu_footer_size = 2;
        } else {
            apdu_footer_size = 0;
        }
        // ヘッダーからデータ長を抽出
        data_in_frame_size = (size_t)(
            (data[APDU_HEADER_SIZE + 0] << 8 & 0xFF00) +
             data[APDU_HEADER_SIZE + 1]
            );
        
    } else {
        // Short APDUと判断し、Lc、Leバイトの長さを設定
        apdu_header_size = APDU_HEADER_SIZE;
        if (is_last_nfc_frame(apdu)) {
            apdu_footer_size = 1;
        } else {
            apdu_footer_size = 0;
        }
        // ヘッダーからデータ長を抽出
        data_in_frame_size = apdu->lc;
    }

    // 実際に受信したデータ長を計算
    //   実際の受信データ長 - ヘッダー長 - フッター長
    real_size = data_size - apdu_header_size - apdu_footer_size;
    if (data_in_frame_size != real_size) {
        // ヘッダーから抽出したデータ長が、
        // 実際に受信したデータ長と異なる場合はエラー
        NRF_LOG_ERROR("APDU size error: data size in header(%d) <> real data size(%d bytes)", data_in_frame_size, real_size);
        return SW_WRONG_LENGTH;
    }
    
    // データを抽出して内部変数に退避
    data_in_frame = data + apdu_header_size;
    memcpy(fido_apdu->data + fido_apdu->data_length, data_in_frame, data_in_frame_size);
    fido_apdu->data_length += data_in_frame_size;

    if (is_last_nfc_frame(apdu)) {
        // ヘッダー項目をコピー
        fido_apdu->CLA = apdu->cla;
        fido_apdu->INS = apdu->ins;
        fido_apdu->P1 = apdu->p1;
        fido_apdu->P2 = apdu->p2;
        fido_apdu->Lc = fido_apdu->data_length;
        // Leは、受信フレームの最終バイトから取得
        if (apdu->lc == 0x00) {
            fido_apdu->Le = (size_t)(
                (data[data_size - 2] << 8 & 0xFF00) +
                 data[data_size - 1]
                );
        } else {
            fido_apdu->Le = (size_t)(data[data_size - 1]);
        }
        NRF_LOG_DEBUG("APDU received: INS (0x%02x) P1(0x%02x) P2(0x%02x) Lc(%d bytes) Le(%d bytes)", 
            fido_apdu->INS, fido_apdu->P1, fido_apdu->P2, fido_apdu->Lc, fido_apdu->Le);
#if NRF_LOG_DEBUG_APDU_BUFF
        print_hexdump_debug(fido_apdu->data, fido_apdu->data_length);
#endif
    }

    return SW_SUCCESS;
}

static void perform_fido_ctap2_message(uint8_t *data, size_t data_size)
{
    if (selected_app != APP_FIDO) {
        nfc_fido_send_response(SW_INS_INVALID);
        return;
    }
    
#if NRF_LOG_DEBUG_APDU_FRAME_BUFF
    NRF_LOG_DEBUG("NFC received CTAP2 message(%d bytes):", data_size);
    print_hexdump_debug(data, data_size);
#endif

    // APDUエンコーディングの判定を行う。
    uint16_t status_word = process_with_nfc_header(data, data_size, &fido_apdu);
    if (status_word != SW_SUCCESS) {
        nfc_fido_send_response(status_word);
        return;
    }
    
    if (is_last_nfc_frame((APDU_HEADER *)data)) {
        // 最終フレームの場合、受信したデータについて
        // CTAP2コマンド処理を実行する
        fido_ctap2_command_cbor(TRANSPORT_NFC);

    } else {
        // 最終フレームでない場合は、ここでレスポンスを戻す
        nfc_fido_send_response(status_word);
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
            NRF_LOG_ERROR("Unknown INS 0x%02x", apdu->ins);
			nfc_fido_send_response(SW_INS_INVALID);
            break;
    }
}
