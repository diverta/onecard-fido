/* 
 * File:   fido_hid_receive.c
 * Author: makmorit
 *
 * Created on 2018/11/21, 14:21
 */
#include <stdio.h>
#include <string.h>
//
// プラットフォーム非依存コード
//
#include "ctap2_define.h"
#include "fido_common.h"
#include "fido_define.h"
#include "fido_hid_channel.h"
#include "fido_hid_define.h"
#include "fido_hid_receive.h"
#include "fido_maintenance_define.h"
#include "fido_receive_apdu.h"
#include "fido_hid_send.h"
#include "fido_transport_define.h"
#include "u2f_define.h"

// コマンド実行関数群
#include "fido_command.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(fido_hid_receive);
#endif

// FIDO機能で使用する control point（コマンドバッファ）には、
// 64バイトまで書込み可能とします
static uint8_t  control_point_buffer[USBD_HID_PACKET_SIZE];
static uint16_t control_point_buffer_length;

// リクエストデータに含まれるHIDヘッダー、APDU項目は
// このモジュール内で保持
static HID_HEADER_T hid_header_t;
static FIDO_APDU_T  apdu_t;

//
// リクエストフレーム、およびフレーム数を
// 一時的に保持する領域
// (32フレームまで格納が可能)
//
static uint8_t request_frame_buffer[USBD_HID_MAX_PAYLOAD_SIZE];
static size_t  request_frame_number = 0;

uint8_t fido_hid_receive_header_CMD(void)
{
    return hid_header_t.CMD;
}

uint32_t fido_hid_receive_header_CID(void)
{
    return hid_header_t.CID;
}

uint8_t fido_hid_receive_header_ERROR(void)
{
    return hid_header_t.ERROR;
}

void *fido_hid_receive_apdu(void)
{
    return &apdu_t;
}

uint8_t *fido_hid_receive_apdu_data(void)
{
    return apdu_t.data;
}

uint32_t fido_hid_receive_apdu_Lc(void)
{
    return apdu_t.Lc;
}

static bool extract_and_check_init_packet(HID_HEADER_T *p_hid_header, FIDO_APDU_T *p_apdu)
{
    if (control_point_buffer_length < 3) {
        // 受取ったバイト数が３バイトに満たない場合は、
        // リクエストとして成立しないので終了
        p_hid_header->CMD =   CTAP2_COMMAND_ERROR;
        p_hid_header->ERROR = CTAP1_ERR_INVALID_LENGTH;
        fido_log_error("Invalid request ");
        return false;
    }

    // HIDヘッダー項目を保持
    p_hid_header->CMD = control_point_buffer[0];
    // データ（APDUまたはPINGパケット）の長さを取得
    p_hid_header->LEN = (uint32_t)(
                         (control_point_buffer[1] << 8 & 0xFF00) 
                        + control_point_buffer[2]);
    if (p_hid_header->LEN > USBD_HID_INIT_PAYLOAD_SIZE) {
        // HIDヘッダーに設定されたデータ長が
        // 57文字を超える場合、後続データがあると判断
        p_hid_header->CONT = true;
    } else {
        p_hid_header->CONT = false;
    }
    // SEQは 0xff で初期化しておく
    p_hid_header->SEQ = 0xff;

    // HIDヘッダーだけの場合は、ここで処理を終了
    if (control_point_buffer_length == 3) {
        return true;
    }

    // Control Point参照用の先頭インデックス
    // （＝処理済みバイト数）を保持
    int offset = 3;

    if (p_hid_header->CMD == CTAP2_COMMAND_PING || 
        p_hid_header->CMD == CTAP2_COMMAND_INIT ||
        p_hid_header->CMD == CTAP2_COMMAND_LOCK ||
        p_hid_header->CMD == CTAP2_COMMAND_CBOR ||
        p_hid_header->CMD >= (MNT_COMMAND_BASE | 0x80)) {
        // コマンドがPING、INIT、LOCK、CBOR、管理コマンドの場合は、APDUではないため
        // データ長だけセットしておく
        p_apdu->Lc = p_hid_header->LEN;
    } else {
        // コマンドが上記以外の場合
        // APDUヘッダー項目を編集して保持
        offset += fido_receive_apdu_header(p_apdu, control_point_buffer, control_point_buffer_length, offset);
    }

    if (p_apdu->Lc > APDU_DATA_MAX_LENGTH) {
        // ヘッダーに設定されたデータ長が不正の場合、
        // ここで処理を終了
        fido_log_error("Lc in APDU is too long length (%d) ", p_apdu->Lc);
        p_hid_header->CMD =   CTAP2_COMMAND_ERROR;
        p_hid_header->ERROR = CTAP1_ERR_INVALID_LENGTH;
        return false;
    }

    if (p_apdu->Lc == 0) {
        if (offset < control_point_buffer_length) {
            // データ長が0にもかかわらず、
            // APDUヘッダーの後ろにデータが存在している場合、
            // リクエストとしては不正ではないが、
            // ステータスワード(SW_WRONG_LENGTH)を設定
            fido_log_error("INIT frame has data (%d bytes) while Lc=0 ", control_point_buffer_length - offset);
            p_hid_header->STATUS_WORD = U2F_SW_WRONG_LENGTH;
        }
        // データ長が0の場合は以降の処理を行わない
        return true;
    }

    // データ格納領域を初期化し、アドレスを保持
    fido_receive_apdu_initialize(p_apdu);

    // パケットからAPDU(データ部分)を取り出し、別途確保した領域に格納
    fido_receive_apdu_from_init_frame(p_apdu, control_point_buffer, control_point_buffer_length, offset);

    return true;
}

static void extract_and_check_cont_packet(HID_HEADER_T *p_hid_header, FIDO_APDU_T *p_apdu)
{
    // CMDが空の場合は先頭レコード未送信とみなし
    // エラーと扱う
    if (p_hid_header->CMD == 0x00) {
        fido_log_error("INIT frame not received ");

        p_hid_header->CMD =   CTAP2_COMMAND_ERROR;
        p_hid_header->ERROR = CTAP1_ERR_INVALID_SEQ;
        return;
    }

    // SEQには、分割受信時の２番目以降の
    // レコード連番が入ります
    uint8_t sequence = control_point_buffer[0];

    // シーケンスチェック
    if (sequence == 0) {
        if (p_hid_header->SEQ != 0xff) {
            fido_log_error("Irregular 1st sequence %d ", sequence);

            p_hid_header->CMD =   CTAP2_COMMAND_ERROR;
            p_hid_header->ERROR = CTAP1_ERR_INVALID_SEQ;
            return;
        }
    } else {
        if (sequence != p_hid_header->SEQ+1) {
            fido_log_error("Bad sequence %d-->%d ", 
                p_hid_header->SEQ, sequence);

            p_hid_header->CMD =   CTAP2_COMMAND_ERROR;
            p_hid_header->ERROR = CTAP1_ERR_INVALID_SEQ;
            return;
        }
    }

    // シーケンスを更新
    p_hid_header->SEQ = sequence;

    // コピー先となる領域が初期化されていない場合は終了
    if (p_apdu->data == NULL) {
        fido_log_error("APDU data buffer is not initialized ");
        return;
    }

    // パケットからAPDU(データ部分)を取り出し、別途確保した領域に格納
    fido_receive_apdu_from_cont_frame(p_apdu, control_point_buffer, control_point_buffer_length);
}

static void dump_hid_init_packet(USB_HID_MSG_T *recv_msg)
{
    uint8_t *cid = recv_msg->cid;
    uint8_t  cmd = recv_msg->pkt.init.cmd;
    size_t   len = fido_hid_payload_length_get(recv_msg);

    if (cmd == CTAP2_COMMAND_CBOR) {
        // CBORコマンドである場合を想定したログ
        fido_log_debug("recv INIT frame: CID(0x%08x) CMD(0x%02x) OPTION(0x%02x) LEN(%d)",
            fido_hid_channel_get_cid_from_bytes(cid), cmd, recv_msg->pkt.init.payload[0], len);

    } else {
        fido_log_debug("recv INIT frame: CID(0x%08x) CMD(0x%02x) LEN(%d)",
            fido_hid_channel_get_cid_from_bytes(cid), cmd, len);
    }
}

static void dump_hid_cont_packet(USB_HID_MSG_T *recv_msg)
{
    fido_log_debug("recv CONT frame: CID(0x%08x) SEQ(0x%02x)",
        fido_hid_channel_get_cid_from_bytes(recv_msg->cid), recv_msg->pkt.cont.seq);
}

static void setup_control_point_buffer(uint8_t *payload, size_t payload_size)
{
    // FIDOクライアントから受信したリクエストフレームを、内部バッファに保持
    // （payloadはHIDヘッダーを含まないデータ）
    memset(control_point_buffer, 0, sizeof(control_point_buffer));
    memcpy(control_point_buffer, payload, payload_size);
    control_point_buffer_length = payload_size;
}

static void check_apdu_data_length(void)
{
    if (apdu_t.data_length > apdu_t.Lc) {
        // データヘッダー設定されたデータ長が不正の場合
        // エラーレスポンスメッセージを作成
        fido_log_error("APDU data length(%d) exceeds Lc(%d) ", apdu_t.data_length, apdu_t.Lc);
        hid_header_t.CMD =   CTAP2_COMMAND_ERROR;
        hid_header_t.ERROR = CTAP1_ERR_INVALID_LENGTH;
    }
}

static void extract_request_from_init_frame(uint32_t cid, uint8_t *payload, size_t payload_size)
{
    // リクエストフレームを内部バッファに保持
    setup_control_point_buffer(payload, payload_size);

    if (cid == 0) {
        // CIDが不正の場合
        // エラーレスポンスメッセージを作成
        fido_log_error("Command not allowed on cid 0x%08x", cid);
        hid_header_t.CID =   cid;
        hid_header_t.CMD =   CTAP2_COMMAND_ERROR;
        hid_header_t.ERROR = CTAP1_ERR_INVALID_CHANNEL;
        return;
    }

    // 受信データに設定されたコマンドバイトを取得
    uint8_t recv_cmd = control_point_buffer[0];
    if (cid == USBD_HID_BROADCAST && recv_cmd != CTAP2_COMMAND_INIT && recv_cmd < (MNT_COMMAND_BASE | 0x80)) {
        // CMDがINITまたは管理コマンド以外の場合
        // エラーレスポンスメッセージを作成
        fido_log_error("Command 0x%02x not allowed on cid 0x%08x", recv_cmd, cid);
        hid_header_t.CMD =   CTAP2_COMMAND_ERROR;
        hid_header_t.ERROR = CTAP1_ERR_INVALID_CHANNEL;
        return;
    }

    // 先頭データが２回連続で送信された場合はエラー
    if (hid_header_t.CONT == true) {
        fido_log_error("INIT frame received again while CONT is expected ");
        hid_header_t.CMD =   CTAP2_COMMAND_ERROR;
        hid_header_t.ERROR = CTAP1_ERR_INVALID_SEQ;
        return;
    }

    // HIDヘッダーとAPDUを初期化
    memset(&hid_header_t, 0, sizeof(HID_HEADER_T));
    memset(&apdu_t, 0, sizeof(FIDO_APDU_T));

    // 初期化されたHIDヘッダーにCIDを再設定
    hid_header_t.CID = cid;

    // 先頭パケットに対する処理を行う
    extract_and_check_init_packet(&hid_header_t, &apdu_t);
    
    // データ長のチェックを行う
    check_apdu_data_length();
}

static void extract_request_from_cont_frame(uint32_t cid, uint8_t *payload, size_t payload_size)
{
    // リクエストフレームを内部バッファに保持
    setup_control_point_buffer(payload, payload_size);

    // 後続パケットに対する処理を行う
    extract_and_check_cont_packet(&hid_header_t, &apdu_t);
    hid_header_t.CONT = false;
    
    // データ長のチェックを行う
    check_apdu_data_length();
}

static void extract_request_from_received_frames(uint8_t *request_frame_buffer, size_t request_frame_number)
{
    static size_t pos;
    static size_t payload_len;
    static uint32_t cid;

    for (int n = 0; n < request_frame_number; n++) {
        USB_HID_MSG_T *req = (USB_HID_MSG_T *)(request_frame_buffer + n * USBD_HID_PACKET_SIZE);
        if (n == 0) {
            dump_hid_init_packet(req);

            // payload長を取得し、リクエストデータ領域に格納
            payload_len = fido_hid_payload_length_get(req);
            pos = (payload_len < USBD_HID_INIT_PAYLOAD_SIZE) ? payload_len : USBD_HID_INIT_PAYLOAD_SIZE;

            // CIDを保持
            cid = fido_hid_channel_get_cid_from_bytes(req->cid);

            // リクエストデータのチェックと格納
            // （引数にはHIDヘッダーを含まないデータを渡す）
            extract_request_from_init_frame(cid, (uint8_t *)&req->pkt.init, pos + 3);

        } else {
            dump_hid_cont_packet(req);

            // リクエストデータ領域に格納
            size_t remain = payload_len - pos;
            size_t cnt = (remain < USBD_HID_CONT_PAYLOAD_SIZE) ? remain : USBD_HID_CONT_PAYLOAD_SIZE;
            pos += cnt;

            // リクエストデータのチェックと格納
            // （引数にはHIDヘッダーを含まないデータを渡す）
            extract_request_from_cont_frame(cid, (uint8_t *)&req->pkt.cont, cnt + 1);            
        }        
    }
}

static bool is_init_frame(uint8_t cmd, bool remaining)
{
    // hid_fido_receive_request_frame関数で受信した
    // HIDサービスのフレームデータについて、
    // INITフレーム or CONTフレームのいずれであるかのチェックを行う。
    if ((cmd & 0x80) == 0x00) {
        // １バイト目（CMD or SEQ）の先頭ビットが立っていない場合は
        // 無条件でCONTフレームであると判定
        return false;

    } else if (cmd == CTAP2_COMMAND_INIT) {
        // HID INITコマンドの場合は、
        // CONTフレーム受信の途中であっても
        // 無条件でINITフレームであると判定
        // （直前に受信したコマンドの全フレームは無効となる）
        return true;

    } else if (remaining) {
        // １バイト目（CMD or SEQ）の先頭ビットが立っている場合、
        // 受信されていないCONTフレームが残っている時は
        // CONTフレームであると判定
        return false;

    } else {
        // INITフレームであると判定
        return true;
    }
}

bool fido_hid_receive_request_frame(uint8_t *p_buff, size_t size)
{
    static size_t pos;
    static size_t payload_len;
    static bool   remaining = false;

    if (size == 0) {
        return false;
    }

    USB_HID_MSG_T *req = (USB_HID_MSG_T *)p_buff;
    uint8_t cmd = req->pkt.init.cmd;

    if (is_init_frame(cmd, remaining)) {
        // 先頭フレームであればpayload長を取得
        payload_len = fido_hid_payload_length_get(req);
        
        // フレームが最後かどうかを判定するための受信済みデータ長
        pos = (payload_len < USBD_HID_INIT_PAYLOAD_SIZE) ? payload_len : USBD_HID_INIT_PAYLOAD_SIZE;

        // リクエストフレーム全体を一時領域に格納
        memset(request_frame_buffer, 0, USBD_HID_MAX_PAYLOAD_SIZE);
        memcpy(request_frame_buffer, p_buff, size);
        request_frame_number = 1;

    } else {
        if (request_frame_number == 0) {
        // 先頭フレームを受信しない状態で
        // 後続フレームが受信された場合は
        // 単に読み飛ばしとする
            remaining = false;
            return false;
        }

        // 後続フレームの場合
        // フレームが最後かどうかを判定するための受信済みデータ長を更新
        size_t remain = payload_len - pos;
        size_t cnt = (remain < USBD_HID_CONT_PAYLOAD_SIZE) ? remain : USBD_HID_CONT_PAYLOAD_SIZE;
        pos += cnt;

        // リクエストフレーム全体を一時領域に格納
        memcpy(request_frame_buffer + request_frame_number * USBD_HID_PACKET_SIZE, 
            p_buff, size);
        request_frame_number++;
    }

    // リクエストデータを全て受信したらtrueを戻す
    if (pos == payload_len) {
        remaining = false;
        return true;
    } else {
        remaining = true;
        return false;
    }
}

void fido_hid_receive_on_request_received(void)
{
    // 受信したフレームから、リクエストデータを取得し、
    // 同時に内容をチェックする
    extract_request_from_received_frames(request_frame_buffer, request_frame_number);
    
    // 受信リクエストフレーム数を初期化
    request_frame_number = 0;

    uint8_t cmd = fido_hid_receive_header_CMD();
    if (cmd == U2F_COMMAND_ERROR) {
        // チェック結果がNGの場合はここで処理中止
        fido_hid_send_status_response(U2F_COMMAND_ERROR, fido_hid_receive_header_ERROR());
        return;
    }

    uint32_t cid = fido_hid_receive_header_CID();
    uint32_t cid_curr = fido_hid_channel_current_cid();
    if (cmd != CTAP2_COMMAND_INIT && cmd < (MNT_COMMAND_BASE | 0x80) && cid != cid_curr) {
        // INIT以外のコマンドを受信したら、
        // INITで発行されたCIDであるかどうかチェックし、
        // 違っている場合はエラー CTAP1_ERR_INVALID_CHANNEL をレスポンス
        fido_log_error("Command 0x%02x not allowed on unknown cid 0x%08x", cmd, cid);
        fido_hid_send_status_response(U2F_COMMAND_ERROR, CTAP1_ERR_INVALID_CHANNEL);
        return;
    }

    uint32_t cid_for_lock = fido_hid_channel_lock_cid();
    if (cid != cid_for_lock && cid_for_lock != 0) {
        // ロック対象CID以外からコマンドを受信したら
        // エラー CTAP1_ERR_CHANNEL_BUSY をレスポンス
        fido_log_error("Command 0x%02x not allowed on non-locked cid 0x%08x", cmd, cid);
        fido_hid_send_status_response(U2F_COMMAND_ERROR, CTAP1_ERR_CHANNEL_BUSY);
        return;
    }
    
    // データ受信後に実行すべき処理
    fido_command_on_hid_request_receive_completed();
}
