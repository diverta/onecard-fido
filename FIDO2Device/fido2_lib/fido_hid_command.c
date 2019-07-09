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
#include "fido_ctap2_command.h"
#include "fido_hid_channel.h"
#include "fido_hid_receive.h"
#include "fido_hid_send.h"
#include "fido_maintenance.h"
#include "fido_u2f_command.h"
#include "u2f.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

static void hid_fido_command_ping(void)
{
    // PINGの場合は
    // リクエストのHIDヘッダーとデータを編集せず
    // レスポンスとして戻す（エコーバック）
    uint32_t cid = fido_hid_receive_header()->CID;
    uint8_t  cmd = fido_hid_receive_header()->CMD;
    uint8_t *data = fido_hid_receive_apdu()->data;
    size_t   length = fido_hid_receive_apdu()->data_length;
    fido_hid_send_command_response(cid, cmd, data, length);
}

static void hid_fido_command_wink(void)
{
    // ステータスなしでレスポンスする
    uint32_t cid = fido_hid_receive_header()->CID;
    uint8_t  cmd = fido_hid_receive_header()->CMD;
    fido_hid_send_command_response_no_payload(cid, cmd);
}

static void hid_fido_command_lock(void)
{
    // ロックコマンドのパラメーターを取得する
    uint32_t cid = fido_hid_receive_header()->CID;
    uint8_t  cmd = fido_hid_receive_header()->CMD;
    uint8_t  lock_param = fido_hid_receive_apdu()->data[0];

    if (lock_param > 0) {
        // パラメーターが指定されていた場合
        // ロック対象CIDを設定
        fido_lock_channel_start(cid, lock_param);

    } else {
        // CIDのロックを解除
        fido_lock_channel_cancel();
    }

    // ステータスなしでレスポンスする
    fido_hid_send_command_response_no_payload(cid, cmd);
}

void fido_hid_command_send_status_response(uint8_t cmd, uint8_t status_code) 
{
    // U2F ERRORコマンドに対応する
    // レスポンスデータを送信パケットに設定し送信
    uint32_t cid = fido_hid_receive_header()->CID;
    fido_hid_send_command_response_no_callback(cid, cmd, status_code);

    // 処理タイムアウト監視を停止
    fido_comm_interval_timer_stop();

    // アイドル時点滅処理を開始
    fido_idling_led_blink_start();
}

void fido_hid_command_on_report_received(uint8_t *request_frame_buffer, size_t request_frame_number)
{
    // 受信したフレームから、リクエストデータを取得し、
    // 同時に内容をチェックする
    fido_hid_receive_request_data(request_frame_buffer, request_frame_number);

    uint8_t cmd = fido_hid_receive_header()->CMD;
    if (cmd == U2F_COMMAND_ERROR) {
        // チェック結果がNGの場合はここで処理中止
        fido_hid_command_send_status_response(U2F_COMMAND_ERROR, fido_hid_receive_header()->ERROR);
        return;
    }

    // ユーザー所在確認待ちが行われている場合
    if (cmd == CTAP2_COMMAND_CANCEL) {
        // キャンセルコマンドの場合は
        // 所在確認待ちをキャンセルしたうえで
        // キャンセルレスポンスを戻す
        fido_ctap2_command_cancel();
        return;
    } else {
        // 他のコマンドの場合は所在確認待ちをキャンセル
        fido_ctap2_command_tup_cancel();
    }

    uint32_t cid = fido_hid_receive_header()->CID;
    uint32_t cid_for_lock = fido_lock_channel_cid();
    if (cid != cid_for_lock && cid_for_lock != 0) {
        // ロック対象CID以外からコマンドを受信したら
        // エラー CTAP1_ERR_CHANNEL_BUSY をレスポンス
        fido_hid_command_send_status_response(U2F_COMMAND_ERROR, CTAP1_ERR_CHANNEL_BUSY);
        return;
    }

    // データ受信後に実行すべき処理を判定
    switch (cmd) {
#if CTAP2_SUPPORTED
        case CTAP2_COMMAND_INIT:
            fido_ctap2_command_hid_init();
            break;
        case CTAP2_COMMAND_PING:
            hid_fido_command_ping();
            break;
        case CTAP2_COMMAND_WINK:
            hid_fido_command_wink();
            break;
        case CTAP2_COMMAND_LOCK:
            hid_fido_command_lock();
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
            fido_maintenance_command();
            break;
        default:
            // 不正なコマンドであるため
            // エラーレスポンスを送信
            fido_log_error("Invalid command (0x%02x) ", cmd);
            fido_hid_command_send_status_response(U2F_COMMAND_ERROR, CTAP1_ERR_INVALID_COMMAND);
            break;
    }
}

void fido_hid_command_on_report_completed(void)
{
    // FIDO機能レスポンスの
    // 全フレーム送信完了時の処理を実行
    // 
    // 処理タイムアウト監視を停止
    fido_comm_interval_timer_stop();

    // 全フレーム送信後に行われる後続処理を実行
    uint8_t cmd = fido_hid_receive_header()->CMD;
    switch (cmd) {
        case CTAP2_COMMAND_INIT:
            fido_log_info("CTAPHID_INIT end");
            break;
        case CTAP2_COMMAND_PING:
            fido_log_info("CTAPHID_PING end");
            break;
        case U2F_COMMAND_MSG:
            fido_u2f_command_msg_response_sent();
            break;
        case CTAP2_COMMAND_CBOR:
            fido_ctap2_command_cbor_response_sent();
            break;
        case MNT_COMMAND_ERASE_SKEY_CERT:
        case MNT_COMMAND_INSTALL_SKEY_CERT:
            fido_maintenance_command_report_sent();
            break;
        default:
            break;
    }

    if (fido_command_do_abort()) {
        // レスポンス完了後の処理を停止させる場合はここで終了
        return;
    }

    // アイドル時点滅処理を開始
    fido_idling_led_blink_start();
}

void fido_hid_command_on_report_started(void) 
{
    // FIDO機能リクエストの
    // 先頭フレーム受信時の処理を実行
    // 
    // 処理タイムアウト監視を開始
    fido_comm_interval_timer_start();

    // アイドル時点滅処理を停止
    fido_idling_led_blink_stop();
}
