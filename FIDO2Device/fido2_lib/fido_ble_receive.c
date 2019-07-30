/* 
 * File:   fido_ble_receive.c
 * Author: makmorit
 *
 * Created on 2019/06/26, 11:32
 */
#include <string.h>
//
// プラットフォーム非依存コード
//
#include "u2f.h"
#include "fido_ble_receive.h"
#include "fido_ble_send.h"
#include "fido_receive_apdu.h"

// コマンド実行関数群
#include "fido_command.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// for debug hex dump data
#define NRF_LOG_HEXDUMP_DEBUG_PACKET false

// u2f control point（コマンドバッファ）には、
// 64バイトまで書込み可能とします
#define U2F_MAX_RECV_CHAR_LEN 64
static uint8_t  control_point_buffer[U2F_MAX_RECV_CHAR_LEN];
static uint16_t control_point_buffer_length;

// 無通信タイムアウトタイマーが開始後、
// 受信したリクエストフレーム数を
// このモジュール内で保持
static uint8_t received_frame_count;

// リクエストデータに含まれるBLEヘッダー、APDU項目は
// このモジュール内で保持
static BLE_HEADER_T ble_header_t;
static FIDO_APDU_T  apdu_t;

BLE_HEADER_T *fido_ble_receive_header(void)
{
    return &ble_header_t;
}

FIDO_APDU_T *fido_ble_receive_apdu(void)
{
    return &apdu_t;
}

void fido_ble_receive_frame_count_clear(void)
{
    // 受信フレーム数をリセット
    received_frame_count = 0;
}

uint8_t fido_ble_receive_frame_count(void)
{
    return received_frame_count;
}

void fido_ble_receive_init(void)
{
    // コマンド／リクエストデータ格納領域を初期化
    memset(control_point_buffer, 0x00, sizeof(control_point_buffer));
    control_point_buffer_length = 0;
    memset(&ble_header_t, 0x00, sizeof(BLE_HEADER_T));
    memset(&apdu_t, 0x00, sizeof(FIDO_APDU_T));
}

static bool is_valid_command(uint8_t command)
{
    if (command == U2F_COMMAND_PING) {
        return true;
    } else if (command == U2F_COMMAND_MSG) {
        return true;
    } else {
        return false;
    }
}

static bool u2f_request_receive_leading_packet(BLE_HEADER_T *p_ble_header, FIDO_APDU_T *p_apdu)
{
    if (control_point_buffer_length < 3) {
        // 受取ったバイト数が３バイトに満たない場合は、
        // リクエストとして成立しないので終了
        p_ble_header->CMD = U2F_COMMAND_ERROR;
        p_ble_header->ERROR = CTAP1_ERR_INVALID_LENGTH;
        fido_log_error("u2f_request_receive: invalid request ");
        return false;
    }

    // BLEヘッダー項目を保持
    p_ble_header->CMD = control_point_buffer[0];
    // データ（APDUまたはPINGパケット）の長さを取得
    p_ble_header->LEN = (uint32_t)(
                         (control_point_buffer[1] << 8 & 0xFF00) 
                        + control_point_buffer[2]);
    p_ble_header->SEQ = 0xff;

    fido_log_debug("recv INIT frame: CMD(0x%02x) LEN(%d) SEQ(%d) ", 
        p_ble_header->CMD, p_ble_header->LEN, p_ble_header->SEQ);

    if (is_valid_command(p_ble_header->CMD) == false) {
        // BLEヘッダーに設定されたコマンドが不正の場合、
        // ここで処理を終了
        fido_log_error("u2f_request_receive: invalid command (0x%02x) ", p_ble_header->CMD);
        p_ble_header->CMD = U2F_COMMAND_ERROR;
        p_ble_header->ERROR = CTAP1_ERR_INVALID_COMMAND;
        return false;
    }

    if (p_ble_header->LEN > U2F_MAX_RECV_CHAR_LEN - 3) {
        // BLEヘッダーに設定されたデータ長が
        // 61文字を超える場合、後続データがあると判断
        fido_log_debug("u2f_request_receive: CONT frame will receive ");
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

    if (p_ble_header->CMD == U2F_COMMAND_PING) {
        // コマンドがPINGの場合
        // データ長だけセットしておく
        p_apdu->Lc = p_ble_header->LEN;
    } else {
        p_apdu->CLA = control_point_buffer[offset];
        if (p_apdu->CLA != 0x00) {
            // CLA部（control pointの先頭から4バイトめ）が
            // 0x00以外の場合は、CTAP2とみなし、
            // CLA部およびデータ長だけをセットしておく
            p_apdu->Lc = p_ble_header->LEN;
            fido_log_debug("CTAP2 command(0x%02x) CBOR size(%d) ", 
                p_apdu->CLA, p_apdu->Lc - 1);
        } else {
            // コマンドがPING以外で、U2Fの場合
            // APDUヘッダー項目を編集して保持
            offset += fido_receive_apdu_header(p_apdu, control_point_buffer, control_point_buffer_length, offset);
        }
    }

    if (p_apdu->Lc > APDU_DATA_MAX_LENGTH) {
        // ヘッダーに設定されたデータ長が不正の場合、
        // ここで処理を終了
        fido_log_error("u2f_request_receive: too long length (%d) ", p_apdu->Lc);
        p_ble_header->CMD = U2F_COMMAND_ERROR;
        p_ble_header->ERROR = CTAP1_ERR_INVALID_LENGTH;
        return false;
    }

    if (p_apdu->Lc == 0) {
        if (offset < control_point_buffer_length) {
            // データ長が0にもかかわらず、
            // APDUヘッダーの後ろにデータが存在している場合、
            // リクエストとしては不正ではないが、
            // ステータスワード(SW_WRONG_LENGTH)を設定
            fido_log_error("INIT frame has data (%d bytes) while Lc=0 ", control_point_buffer_length - offset);
            p_ble_header->STATUS_WORD = U2F_SW_WRONG_LENGTH;
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


static void u2f_request_receive_following_packet(BLE_HEADER_T *p_ble_header, FIDO_APDU_T *p_apdu)
{
    // CMDが空の場合は先頭レコード未送信とみなし
    // エラーと扱う
    if (p_ble_header->CMD == 0x00) {
        fido_log_error("INIT frame not received ");

        ble_header_t.CMD = U2F_COMMAND_ERROR;
        ble_header_t.ERROR = CTAP1_ERR_INVALID_SEQ;
        return;
    }

    // SEQには、分割受信時の２番目以降の
    // レコード連番が入ります
    uint8_t sequence = control_point_buffer[0];

    // シーケンスチェック
    if (sequence == 0) {
        if (p_ble_header->SEQ != 0xff) {
            fido_log_error("Irregular 1st sequence %d ", sequence);

            ble_header_t.CMD = U2F_COMMAND_ERROR;
            ble_header_t.ERROR = CTAP1_ERR_INVALID_SEQ;
            return;
        }
    } else {
        if (sequence != p_ble_header->SEQ+1) {
            fido_log_error("Bad sequence %d-->%d ", 
                p_ble_header->SEQ, sequence);

            ble_header_t.CMD = U2F_COMMAND_ERROR;
            ble_header_t.ERROR = CTAP1_ERR_INVALID_SEQ;
            return;
        }
    }

    // シーケンスを更新
    p_ble_header->SEQ = sequence;

    fido_log_debug("recv CONT frame: CMD(0x%02x) LEN(%d) SEQ(%d) ", 
        p_ble_header->CMD, p_ble_header->LEN, p_ble_header->SEQ);

    // コピー先となる領域が初期化されていない場合は終了
    if (p_apdu->data == NULL) {
        fido_log_error("p_apdu->data is not initialized ");
        return;
    }

    // パケットからAPDU(データ部分)を取り出し、別途確保した領域に格納
    fido_receive_apdu_from_cont_frame(p_apdu, control_point_buffer, control_point_buffer_length);
}

bool fido_ble_receive_control_point(uint8_t *data, uint16_t length)
{
    // U2Fクライアントから受信したリクエストデータを、
    // 内部バッファに保持
    memset(control_point_buffer, 0, sizeof(control_point_buffer));
    memcpy(control_point_buffer, data, length);
    control_point_buffer_length = length;

#if NRF_LOG_HEXDUMP_DEBUG_PACKET
    fido_log_debug("ble_u2f_control_point_receive length=%u ", control_point_buffer_length);
    fido_log_print_hexdump_debug(control_point_buffer, control_point_buffer_length);
#endif

    if (control_point_buffer[0] & 0x80) {
        // 先頭データが２回連続で送信された場合はエラー
        if ((ble_header_t.CMD & 0x80) && ble_header_t.CONT == true) {
            fido_log_error("INIT frame received again while CONT is expected ");
            ble_header_t.CMD = U2F_COMMAND_ERROR;
            ble_header_t.ERROR = CTAP1_ERR_INVALID_SEQ;

        } else {
            // BLEヘッダーとAPDUを初期化
            memset(&ble_header_t, 0, sizeof(BLE_HEADER_T));
            memset(&apdu_t, 0, sizeof(FIDO_APDU_T));

            // 先頭パケットに対する処理を行う
            u2f_request_receive_leading_packet(&ble_header_t, &apdu_t);
        }

    } else {
        // 後続パケットに対する処理を行う
        u2f_request_receive_following_packet(&ble_header_t, &apdu_t);
        ble_header_t.CONT = false;
    }

    if (apdu_t.data_length > apdu_t.Lc) {
        // データヘッダー設定されたデータ長が不正の場合
        // エラーレスポンスメッセージを作成
        fido_log_error("apdu data length(%d) exceeds Lc(%d) ", apdu_t.data_length, apdu_t.Lc);
        ble_header_t.CMD = U2F_COMMAND_ERROR;
        ble_header_t.ERROR = CTAP1_ERR_INVALID_LENGTH;
    }

    // 受信フレーム数をカウントアップ
    received_frame_count++;

    if (apdu_t.data_length == apdu_t.Lc) {
        // 全ての受信データが完備したらtrueを戻す
        fido_log_debug("apdu data received(%d bytes)", apdu_t.data_length);
        return true;

    } else if (ble_header_t.CMD == U2F_COMMAND_ERROR) {
        // リクエストデータの検査中にエラーが確認された場合、
        // エラーレスポンス実行のため、trueを戻す
        return true;
        
    } else {
        // データが完備していなければfalseを戻す
        return false;
    }
}

static bool invalid_command_in_pairing_mode(uint8_t cmd, uint8_t ins)
{
    if (fido_ble_pairing_mode_get()) {
        if (cmd == U2F_COMMAND_MSG && ins == U2F_INS_INSTALL_PAIRING) {
            // ペアリングモード時に実行できる
            // ペアリング機能なら false を戻す
            return false;
        } else {
            // ペアリングモード時に実行できない機能なら 
            // true を戻す
            return true;
        }
    } else {
        // 非ペアリングモード時は常に false を戻す
        return false;
    }
}

void fido_ble_receive_on_request_received(void)
{
    // BLEヘッダー、APDUの参照を取得
    BLE_HEADER_T *p_ble_header = fido_ble_receive_header();
    FIDO_APDU_T  *p_apdu = fido_ble_receive_apdu();

    if (p_ble_header->CMD == U2F_COMMAND_ERROR) {
        // リクエストデータの検査中にエラーが確認された場合、
        // エラーレスポンスを戻す
        fido_ble_send_status_response(U2F_COMMAND_ERROR, fido_ble_receive_header()->ERROR);
        return;
    }
    
    // ペアリングモード時はペアリング以外の機能を実行できないようにするため
    // エラーステータスワード (0x9601) を戻す
    if (invalid_command_in_pairing_mode(p_ble_header->CMD, p_apdu->INS)) {
        fido_ble_send_status_word(p_ble_header->CMD, 0x9601);
        return;
    }
    
    // データ受信後に実行すべき処理
    fido_command_on_request_receive_completed(TRANSPORT_BLE);
}
