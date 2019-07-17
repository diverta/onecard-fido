/* 
 * File:   fido_hid_command.c
 * Author: makmorit
 *
 * Created on 2018/11/21, 14:21
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//
// プラットフォーム非依存コード
//
#include "ctap2_common.h"
#include "fido_command.h"
#include "fido_ctap2_command.h"
#include "fido_hid_channel.h"
#include "fido_hid_receive.h"
#include "fido_hid_send.h"
#include "fido_maintenance.h"
#include "fido_u2f_command.h"
#include "u2f.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

void fido_hid_command_on_report_received(uint8_t *request_frame_buffer, size_t request_frame_number)
{
    // 受信したフレームから、リクエストデータを取得し、
    // 同時に内容をチェックする
    fido_hid_receive_request_data(request_frame_buffer, request_frame_number);

    uint8_t cmd = fido_hid_receive_header()->CMD;
    if (cmd == U2F_COMMAND_ERROR) {
        // チェック結果がNGの場合はここで処理中止
        fido_hid_send_status_response(U2F_COMMAND_ERROR, fido_hid_receive_header()->ERROR);
        return;
    }

    // ユーザー所在確認待ちが行われている場合
    if (cmd == CTAP2_COMMAND_CANCEL) {
        // キャンセルコマンドの場合は
        // 所在確認待ちをキャンセルしたうえで
        // キャンセルレスポンスを戻す
        fido_ctap2_command_cancel();
        return;
    }

    uint32_t cid = fido_hid_receive_header()->CID;
    uint32_t cid_for_lock = fido_lock_channel_cid();
    if (cid != cid_for_lock && cid_for_lock != 0) {
        // ロック対象CID以外からコマンドを受信したら
        // エラー CTAP1_ERR_CHANNEL_BUSY をレスポンス
        fido_hid_send_status_response(U2F_COMMAND_ERROR, CTAP1_ERR_CHANNEL_BUSY);
        return;
    }

    // データ受信後に実行すべき処理を判定
    switch (cmd) {
#if CTAP2_SUPPORTED
        case CTAP2_COMMAND_INIT:
            fido_ctap2_command_hid_init();
            break;
        case CTAP2_COMMAND_PING:
            fido_ctap2_command_ping();
            break;
        case CTAP2_COMMAND_WINK:
            fido_ctap2_command_wink();
            break;
        case CTAP2_COMMAND_LOCK:
            fido_ctap2_command_lock();
            break;
#else
        case U2F_COMMAND_HID_INIT:
            fido_u2f_command_hid_init();
            break;
#endif
        case U2F_COMMAND_MSG:
            fido_u2f_command_msg(TRANSPORT_HID);
            break;
        case CTAP2_COMMAND_CBOR:
            fido_ctap2_command_cbor(TRANSPORT_HID);
            break;
        case MNT_COMMAND_ERASE_SKEY_CERT:
        case MNT_COMMAND_INSTALL_SKEY_CERT:
        case MNT_COMMAND_GET_FLASH_STAT:
            fido_maintenance_command();
            break;
        default:
            // 不正なコマンドであるため
            // エラーレスポンスを送信
            fido_log_error("Invalid command (0x%02x) ", cmd);
            fido_hid_send_status_response(U2F_COMMAND_ERROR, CTAP1_ERR_INVALID_COMMAND);
            break;
    }
}
