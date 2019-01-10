/* 
 * File:   hid_fido_receive.c
 * Author: makmorit
 *
 * Created on 2018/11/21, 14:21
 */
#include <stdio.h>
#include "fido_request_apdu.h"
#include "hid_fido_command.h"
#include "hid_fido_receive.h"
#include "usbd_hid_common.h"

// for logging informations
#define NRF_LOG_MODULE_NAME hid_fido_receive
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// 使用するコマンド／ステータスの読替え
#include "u2f.h"
#include "ctap2_common.h"
#if CTAP2_SUPPORTED
#define FIDO_COMMAND_ERROR   CTAP2_COMMAND_ERROR
#define FIDO_COMMAND_PING    CTAP2_COMMAND_PING
#define FIDO_COMMAND_INIT    CTAP2_COMMAND_INIT
#else
#define FIDO_COMMAND_ERROR   U2F_COMMAND_ERROR
#define FIDO_COMMAND_PING    U2F_COMMAND_PING
#define FIDO_COMMAND_INIT    U2F_COMMAND_HID_INIT
#endif
#define FIDO_COMMAND_CBOR    CTAP2_COMMAND_CBOR

// FIDO機能で使用する control point（コマンドバッファ）には、
// 64バイトまで書込み可能とします
static uint8_t  control_point_buffer[64];
static uint16_t control_point_buffer_length;

// リクエストデータに含まれるHIDヘッダー、APDU項目は
// このモジュール内で保持
static HID_HEADER_T hid_header_t;
static FIDO_APDU_T  apdu_t;

HID_HEADER_T *hid_fido_receive_hid_header(void)
{
    return &hid_header_t;
}

FIDO_APDU_T *hid_fido_receive_apdu(void)
{
    return &apdu_t;
}

static bool extract_and_check_init_packet(HID_HEADER_T *p_hid_header, FIDO_APDU_T *p_apdu)
{
    if (control_point_buffer_length < 3) {
        // 受取ったバイト数が３バイトに満たない場合は、
        // リクエストとして成立しないので終了
        p_hid_header->CMD =   FIDO_COMMAND_ERROR;
        p_hid_header->ERROR = CTAP1_ERR_INVALID_LENGTH;
        NRF_LOG_ERROR("Invalid request ");
        return false;
    }

    // HIDヘッダー項目を保持
    p_hid_header->CMD = control_point_buffer[0];
    // データ（APDUまたはPINGパケット）の長さを取得
    p_hid_header->LEN = (uint32_t)(
                         (control_point_buffer[1] << 8 & 0xFF00) 
                        + control_point_buffer[2]);
    p_hid_header->SEQ = 0xff;

    NRF_LOG_DEBUG("INIT frame: CMD(0x%02x) LEN(%d) SEQ(%d) ", 
        p_hid_header->CMD, p_hid_header->LEN, p_hid_header->SEQ);

    if (hid_fido_command_is_valid(p_hid_header->CMD) == false) {
        // HIDヘッダーに設定されたコマンドが不正の場合、
        // ここで処理を終了
        NRF_LOG_ERROR("Invalid command (0x%02x) ", p_hid_header->CMD);
        p_hid_header->CMD =   FIDO_COMMAND_ERROR;
        p_hid_header->ERROR = CTAP1_ERR_INVALID_COMMAND;
        return false;
    }

    if (p_hid_header->LEN > USBD_HID_INIT_PAYLOAD_SIZE) {
        // HIDヘッダーに設定されたデータ長が
        // 57文字を超える場合、後続データがあると判断
        NRF_LOG_DEBUG("CONT frame will receive ");
        p_hid_header->CONT = true;
    } else {
        p_hid_header->CONT = false;
    }
    
    // HIDヘッダーだけの場合は、ここで処理を終了
    if (control_point_buffer_length == 3) {
        return true;
    }

    // Control Point参照用の先頭インデックス
    // （＝処理済みバイト数）を保持
    int offset = 3;

    if (p_hid_header->CMD == FIDO_COMMAND_PING || 
        p_hid_header->CMD == FIDO_COMMAND_INIT ||
        p_hid_header->CMD == FIDO_COMMAND_CBOR) {
        // コマンドがPING、INIT、CBORの場合は、APDUではないため
        // データ長だけセットしておく
        p_apdu->Lc = p_hid_header->LEN;
    } else {
        // コマンドがPING以外の場合
        // APDUヘッダー項目を編集して保持
        offset += fido_request_apdu_header(p_apdu, control_point_buffer, control_point_buffer_length, offset);
    }

    if (p_apdu->Lc > APDU_DATA_MAX_LENGTH) {
        // ヘッダーに設定されたデータ長が不正の場合、
        // ここで処理を終了
        NRF_LOG_ERROR("Lc in APDU is too long length (%d) ", p_apdu->Lc);
        p_hid_header->CMD =   FIDO_COMMAND_ERROR;
        p_hid_header->ERROR = CTAP1_ERR_INVALID_LENGTH;
        return false;
    }

    if (p_apdu->Lc == 0) {
        if (offset < control_point_buffer_length) {
            // データ長が0にもかかわらず、
            // APDUヘッダーの後ろにデータが存在している場合、
            // リクエストとしては不正ではないが、
            // ステータスワード(SW_WRONG_LENGTH)を設定
            NRF_LOG_ERROR("INIT frame has data (%d bytes) while Lc=0 ", control_point_buffer_length - offset);
            p_hid_header->STATUS_WORD = U2F_SW_WRONG_LENGTH;
        }
        // データ長が0の場合は以降の処理を行わない
        return true;
    }

    // データ格納領域を初期化し、アドレスを保持
    fido_request_apdu_initialize(p_apdu);

    // パケットからAPDU(データ部分)を取り出し、別途確保した領域に格納
    fido_request_apdu_from_init_frame(p_apdu, control_point_buffer, control_point_buffer_length, offset);

    return true;
}

static void extract_and_check_cont_packet(HID_HEADER_T *p_hid_header, FIDO_APDU_T *p_apdu)
{
    // CMDが空の場合は先頭レコード未送信とみなし
    // エラーと扱う
    if (p_hid_header->CMD == 0x00) {
        NRF_LOG_ERROR("INIT frame not received ");

        p_hid_header->CMD =   FIDO_COMMAND_ERROR;
        p_hid_header->ERROR = CTAP1_ERR_INVALID_SEQ;
        return;
    }

    // SEQには、分割受信時の２番目以降の
    // レコード連番が入ります
    uint8_t sequence = control_point_buffer[0];

    // シーケンスチェック
    if (sequence == 0) {
        if (p_hid_header->SEQ != 0xff) {
            NRF_LOG_ERROR("Irregular 1st sequence %d ", sequence);

            p_hid_header->CMD =   FIDO_COMMAND_ERROR;
            p_hid_header->ERROR = CTAP1_ERR_INVALID_SEQ;
            return;
        }
    } else {
        if (sequence != p_hid_header->SEQ+1) {
            NRF_LOG_ERROR("Bad sequence %d-->%d ", 
                p_hid_header->SEQ, sequence);

            p_hid_header->CMD =   FIDO_COMMAND_ERROR;
            p_hid_header->ERROR = CTAP1_ERR_INVALID_SEQ;
            return;
        }
    }

    // シーケンスを更新
    p_hid_header->SEQ = sequence;

    NRF_LOG_DEBUG("CONT frame: CMD(0x%02x) LEN(%d) SEQ(%d) ", 
        p_hid_header->CMD, p_hid_header->LEN, p_hid_header->SEQ);

    // コピー先となる領域が初期化されていない場合は終了
    if (p_apdu->data == NULL) {
        NRF_LOG_ERROR("APDU data buffer is not initialized ");
        return;
    }

    // パケットからAPDU(データ部分)を取り出し、別途確保した領域に格納
    fido_request_apdu_from_cont_frame(p_apdu, control_point_buffer, control_point_buffer_length);
}

static void extract_and_check_request_data(uint32_t cid, uint8_t *payload, size_t payload_size)
{
    // FIDOクライアントから受信したリクエストデータを、内部バッファに保持
    // （payloadはHIDヘッダーを含まないデータ）
    memset(control_point_buffer, 0, sizeof(control_point_buffer));
    memcpy(control_point_buffer, payload, payload_size);
    control_point_buffer_length = payload_size;

    if (control_point_buffer[0] & 0x80) {
        // 先頭データが２回連続で送信された場合はエラー
        if ((hid_header_t.CMD & 0x80) && hid_header_t.CONT == true) {
            NRF_LOG_ERROR("INIT frame received again while CONT is expected ");
            hid_header_t.CMD =   FIDO_COMMAND_ERROR;
            hid_header_t.ERROR = CTAP1_ERR_INVALID_SEQ;

        } else {
            // HIDヘッダーとAPDUを初期化
            memset(&hid_header_t, 0, sizeof(HID_HEADER_T));
            memset(&apdu_t, 0, sizeof(FIDO_APDU_T));

            // 初期化されたHIDヘッダーにCIDを再設定
            hid_header_t.CID = cid;

            // 先頭パケットに対する処理を行う
            extract_and_check_init_packet(&hid_header_t, &apdu_t);
        }

    } else {
        // 後続パケットに対する処理を行う
        extract_and_check_cont_packet(&hid_header_t, &apdu_t);
        hid_header_t.CONT = false;
    }

    if (apdu_t.data_length > apdu_t.Lc) {
        // データヘッダー設定されたデータ長が不正の場合
        // エラーレスポンスメッセージを作成
        NRF_LOG_ERROR("APDU data length(%d) exceeds Lc(%d) ", apdu_t.data_length, apdu_t.Lc);
        hid_header_t.CMD =   FIDO_COMMAND_ERROR;
        hid_header_t.ERROR = CTAP1_ERR_INVALID_LENGTH;
    }
}

void hid_fido_receive_request_data(uint8_t *request_frame_buffer, size_t request_frame_number)
{
    static size_t pos;
    static size_t payload_len;
    static uint32_t cid;

    for (int n = 0; n < request_frame_number; n++) {
        USB_HID_MSG_T *req = (USB_HID_MSG_T *)(request_frame_buffer + n * USBD_HID_PACKET_SIZE);
        if (n == 0) {
            dump_hid_init_packet("Recv ", USBD_HID_PACKET_SIZE, req);

            // payload長を取得し、リクエストデータ領域に格納
            payload_len = get_payload_length(req);
            pos = (payload_len < USBD_HID_INIT_PAYLOAD_SIZE) ? payload_len : USBD_HID_INIT_PAYLOAD_SIZE;

            // CIDを保持
            cid = get_CID(req->cid);

            // リクエストデータのチェックと格納
            // （引数にはHIDヘッダーを含まないデータを渡す）
            extract_and_check_request_data(cid, (uint8_t *)&req->pkt.init, pos + 3);

            // FIDOリクエスト受信開始時の処理を実行
            hid_fido_command_on_report_started();
            
        } else {
            dump_hid_cont_packet("Recv ", USBD_HID_PACKET_SIZE, req);

            // リクエストデータ領域に格納
            size_t remain = payload_len - pos;
            size_t cnt = (remain < USBD_HID_CONT_PAYLOAD_SIZE) ? remain : USBD_HID_CONT_PAYLOAD_SIZE;
            pos += cnt;

            // リクエストデータのチェックと格納
            // （引数にはHIDヘッダーを含まないデータを渡す）
            extract_and_check_request_data(cid, (uint8_t *)&req->pkt.cont, cnt + 1);            
        }        
    }
}
