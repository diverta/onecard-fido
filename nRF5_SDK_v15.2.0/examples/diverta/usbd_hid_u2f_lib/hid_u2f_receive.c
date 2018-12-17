/* 
 * File:   hid_u2f_receive.c
 * Author: makmorit
 *
 * Created on 2018/11/21, 14:21
 */
#include <stdio.h>
#include "u2f_control_point_apdu.h"
#include "hid_u2f_receive.h"
#include "usbd_hid_common.h"
#include "hid_fido_command.h"

// for logging informations
#define NRF_LOG_MODULE_NAME hid_u2f_receive
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// u2f control point（コマンドバッファ）には、
// 64バイトまで書込み可能とします
static uint8_t  control_point_buffer[64];
static uint16_t control_point_buffer_length;

// リクエストデータに含まれるHIDヘッダー、APDU項目は
// このモジュール内で保持
static HID_HEADER_T hid_header_t;
static U2F_APDU_T   apdu_t;

HID_HEADER_T *hid_u2f_receive_hid_header(void)
{
    return &hid_header_t;
}

U2F_APDU_T *hid_u2f_receive_apdu(void)
{
    return &apdu_t;
}

static bool extract_and_check_init_packet(HID_HEADER_T *p_ble_header, U2F_APDU_T *p_apdu)
{
    if (control_point_buffer_length < 3) {
        // 受取ったバイト数が３バイトに満たない場合は、
        // リクエストとして成立しないので終了
        p_ble_header->CMD = U2F_COMMAND_ERROR;
        p_ble_header->ERROR = U2F_ERR_INVALID_LEN;
        NRF_LOG_ERROR("u2f_request_receive: invalid request ");
        return false;
    }

    // BLEヘッダー項目を保持
    p_ble_header->CMD = control_point_buffer[0];
    // データ（APDUまたはPINGパケット）の長さを取得
    p_ble_header->LEN = (uint32_t)(
                         (control_point_buffer[1] << 8 & 0xFF00) 
                        + control_point_buffer[2]);
    p_ble_header->SEQ = 0xff;

    NRF_LOG_DEBUG("INIT frame: CMD(0x%02x) LEN(%d) SEQ(%d) ", 
        p_ble_header->CMD, p_ble_header->LEN, p_ble_header->SEQ);

    if (hid_u2f_command_is_valid(p_ble_header->CMD) == false) {
        // BLEヘッダーに設定されたコマンドが不正の場合、
        // ここで処理を終了
        NRF_LOG_ERROR("u2f_request_receive: invalid command (0x%02x) ", p_ble_header->CMD);
        p_ble_header->CMD = U2F_COMMAND_ERROR;
        p_ble_header->ERROR = U2F_ERR_INVALID_CMD;
        return false;
    }

    if (p_ble_header->LEN > USBD_HID_INIT_PAYLOAD_SIZE) {
        // HIDヘッダーに設定されたデータ長が
        // 57文字を超える場合、後続データがあると判断
        NRF_LOG_DEBUG("u2f_request_receive: CONT frame will receive ");
        p_ble_header->CONT = true;
    } else {
        p_ble_header->CONT = false;
    }
    
    // BLEヘッダーだけの場合は、ここで処理を終了
    if (control_point_buffer_length == 3) {
        return true;
    }

    // Control Point参照用の先頭インデックス
    // （＝処理済みバイト数）を保持
    int offset = 3;

    if (p_ble_header->CMD == U2F_COMMAND_PING || p_ble_header->CMD == U2F_COMMAND_HID_INIT) {
        // コマンドがPING、INITの場合は、APDUではないため
        // データ長だけセットしておく
        p_apdu->Lc = p_ble_header->LEN;
    } else {
        // コマンドがPING以外の場合
        // APDUヘッダー項目を編集して保持
        offset += u2f_control_point_apdu_header(p_apdu, control_point_buffer, control_point_buffer_length, offset);
    }

    if (p_apdu->Lc > APDU_DATA_MAX_LENGTH) {
        // ヘッダーに設定されたデータ長が不正の場合、
        // ここで処理を終了
        NRF_LOG_ERROR("u2f_request_receive: too long length (%d) ", p_apdu->Lc);
        p_ble_header->CMD = U2F_COMMAND_ERROR;
        p_ble_header->ERROR = U2F_ERR_INVALID_LEN;
        return false;
    }

    if (p_apdu->Lc == 0) {
        if (offset < control_point_buffer_length) {
            // データ長が0にもかかわらず、
            // APDUヘッダーの後ろにデータが存在している場合、
            // リクエストとしては不正ではないが、
            // ステータスワード(SW_WRONG_LENGTH)を設定
            NRF_LOG_ERROR("INIT frame has data (%d bytes) while Lc=0 ", control_point_buffer_length - offset);
            p_ble_header->STATUS_WORD = U2F_SW_WRONG_LENGTH;
        }
        // データ長が0の場合は以降の処理を行わない
        return true;
    }

    // データ格納領域を初期化し、アドレスを保持
    u2f_control_point_apdu_initialize(p_apdu);

    // パケットからAPDU(データ部分)を取り出し、別途確保した領域に格納
    u2f_control_point_apdu_from_leading(p_apdu, control_point_buffer, control_point_buffer_length, offset);

    return true;
}

static void extract_and_check_cont_packet(HID_HEADER_T *p_ble_header, U2F_APDU_T *p_apdu)
{
    // CMDが空の場合は先頭レコード未送信とみなし
    // エラーと扱う
    if (p_ble_header->CMD == 0x00) {
        NRF_LOG_ERROR("INIT frame not received ");

        p_ble_header->CMD = U2F_COMMAND_ERROR;
        p_ble_header->ERROR = U2F_ERR_INVALID_SEQ;
        return;
    }

    // SEQには、分割受信時の２番目以降の
    // レコード連番が入ります
    uint8_t sequence = control_point_buffer[0];

    // シーケンスチェック
    if (sequence == 0) {
        if (p_ble_header->SEQ != 0xff) {
            NRF_LOG_ERROR("Irregular 1st sequence %d ", sequence);

            p_ble_header->CMD = U2F_COMMAND_ERROR;
            p_ble_header->ERROR = U2F_ERR_INVALID_SEQ;
            return;
        }
    } else {
        if (sequence != p_ble_header->SEQ+1) {
            NRF_LOG_ERROR("Bad sequence %d-->%d ", 
                p_ble_header->SEQ, sequence);

            p_ble_header->CMD = U2F_COMMAND_ERROR;
            p_ble_header->ERROR = U2F_ERR_INVALID_SEQ;
            return;
        }
    }

    // シーケンスを更新
    p_ble_header->SEQ = sequence;

    NRF_LOG_DEBUG("CONT frame: CMD(0x%02x) LEN(%d) SEQ(%d) ", 
        p_ble_header->CMD, p_ble_header->LEN, p_ble_header->SEQ);

    // コピー先となる領域が初期化されていない場合は終了
    if (p_apdu->data == NULL) {
        NRF_LOG_ERROR("p_apdu->data is not initialized ");
        return;
    }

    // パケットからAPDU(データ部分)を取り出し、別途確保した領域に格納
    u2f_control_point_apdu_from_following(p_apdu, control_point_buffer, control_point_buffer_length);
}

static void extract_and_check_request_data(uint32_t cid, uint8_t *payload, size_t payload_size)
{
    // U2Fクライアントから受信したリクエストデータを、内部バッファに保持
    // （payloadはHIDヘッダーを含まないデータ）
    memset(control_point_buffer, 0, sizeof(control_point_buffer));
    memcpy(control_point_buffer, payload, payload_size);
    control_point_buffer_length = payload_size;

    if (control_point_buffer[0] & 0x80) {
        // 先頭データが２回連続で送信された場合はエラー
        if ((hid_header_t.CMD & 0x80) && hid_header_t.CONT == true) {
            NRF_LOG_ERROR("INIT frame received again while CONT is expected ");
            hid_header_t.CMD = U2F_COMMAND_ERROR;
            hid_header_t.ERROR = U2F_ERR_INVALID_SEQ;

        } else {
            // BLEヘッダーとAPDUを初期化
            memset(&hid_header_t, 0, sizeof(HID_HEADER_T));
            memset(&apdu_t, 0, sizeof(U2F_APDU_T));

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
        NRF_LOG_ERROR("apdu data length(%d) exceeds Lc(%d) ", apdu_t.data_length, apdu_t.Lc);
        hid_header_t.CMD = U2F_COMMAND_ERROR;
        hid_header_t.ERROR = U2F_ERR_INVALID_LEN;
    }
}

void hid_u2f_receive_request_data(uint8_t *request_frame_buffer, size_t request_frame_number)
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

            // U2F処理スタート時の処理を実行
            hid_u2f_command_on_process_started();
            
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
