/* 
 * File:   fido_command.c
 * Author: makmorit
 *
 * Created on 2019/02/11, 11:31
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
//
// プラットフォーム非依存コード
//
#include "fido_command.h"
#include "fido_u2f_command.h"
#include "fido_ctap2_command.h"
#include "fido_ble_receive.h"
#include "fido_ble_send.h"
#include "fido_development.h"
#include "fido_hid_receive.h"
#include "fido_hid_send.h"
#include "fido_maintenance.h"
#include "fido_maintenance_define.h"
#include "ctap2_common.h"
#include "u2f_define.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(fido_command);
#endif

// ユーザー所在確認タイムアウト（３０秒）
#define USER_PRESENCE_VERIFY_TIMEOUT_MSEC 30000

// ユーザー所在確認待ち状態を示すフラグ
static bool waiting_for_tup = false;

// レスポンス完了後の処理を停止させるフラグ
static bool abort_flag = false;

void fido_command_abort_flag_set(bool flag)
{
    abort_flag = flag;
}

//
// CTAP2、U2Fで共用する各種処理
//
bool fido_command_mainsw_event_handler(void)
{
    // ボタンが短押しされた時の処理を実行
    if (fido_u2f_command_on_mainsw_event() == true) {
        return true;
    }
    return fido_ctap2_command_on_mainsw_event();
}

void fido_command_keepalive_timer_handler(void)
{
    // キープアライブ・コマンドを実行する
    fido_ctap2_command_keepalive_timer_handler();
    fido_u2f_command_keepalive_timer_handler();
}

static void fido_user_presence_verify_timeout_handler(void) 
{
    // ユーザー所在確認タイムアウト発生時の処理を実行
    //
    // モジュール内のフラグをクリア
    fido_u2f_command_tup_cancel();
    fido_ctap2_command_tup_cancel();

    // キープアライブタイマーを停止し、
    // LED制御をアイドル中（秒間２回点滅）に変更
    fido_user_presence_verify_cancel();
}

void fido_user_presence_verify_start_on_reset(void)
{
    // ユーザー所在確認タイムアウト監視を開始
    fido_user_presence_verify_timer_start(USER_PRESENCE_VERIFY_TIMEOUT_MSEC, fido_user_presence_verify_timeout_handler);

    // ユーザー所在確認待ち状態に入る
    waiting_for_tup = true;

    // 赤色LED高速点滅開始
    fido_status_indicator_prompt_reset();
}

void fido_user_presence_verify_start(uint32_t timeout_msec, void *context)
{
    // キープアライブタイマーを開始
    fido_repeat_process_timer_start(timeout_msec, fido_command_keepalive_timer_handler);

    // ユーザー所在確認タイムアウト監視を開始
    fido_user_presence_verify_timer_start(USER_PRESENCE_VERIFY_TIMEOUT_MSEC, fido_user_presence_verify_timeout_handler);

    // ユーザー所在確認待ち状態に入る
    waiting_for_tup = true;

    // LED点滅を開始し、ボタンの押下を待つ
    fido_status_indicator_prompt_tup();
}

void fido_user_presence_verify_cancel(void)
{
    //
    // 以下のいずれかの条件で呼び出される
    // ・ユーザー所在確認ボタン押下待ちタイムアウト
    // ・自動認証有効時は、BLEデバイスがスキャンできなかった時
    // ・CTAP2クライアントからキャンセルコマンド送信時
    //
    // キープアライブタイマーを停止する
    fido_repeat_process_timer_stop();
    
    // LED制御をアイドル中（秒間２回点滅）に変更
    fido_status_indicator_idle();

    // ユーザー所在確認待ち完了
    waiting_for_tup = false;
}

void fido_user_presence_verify_end(void)
{
    //
    // 以下のいずれかの条件で呼び出される
    // ・ユーザー所在確認用ボタン押下時
    // ・自動認証有効時は、BLEデバイスがスキャンできた時
    //
    // ユーザー所在確認タイムアウト監視を停止
    fido_user_presence_verify_timer_stop();
    
    // キープアライブタイマーを停止する
    fido_repeat_process_timer_stop();

    // ユーザー所在確認待ち完了
    waiting_for_tup = false;
}

void fido_user_presence_verify_end_message(const char *func_name, bool tup_done)
{
    if (tup_done) {
        // ユーザー所在確認完了時のメッセージを出力
        fido_log_info("%s: completed the test of user presence", func_name);
    } else {
        // ユーザー所在確認省略時のメッセージを出力
        fido_log_info("%s: skipped the test of user presence", func_name);
    }
}

static bool is_waiting_user_presence_verify(TRANSPORT_TYPE transport_type, uint8_t cmd)
{
    if (waiting_for_tup) {
        // キャンセルコマンドの場合は受け付ける
        if (cmd == CTAP2_COMMAND_CANCEL) {
            fido_log_info("Command (0x%02x) accepted while testing user presence ", cmd);
            return false;
        }
        // ユーザー所在確認中は、 ビジーである旨のエラーを戻す
        fido_log_error("Command (0x%02x) cannot perform while testing user presence ", cmd);
        if (transport_type == TRANSPORT_HID) {
            fido_hid_send_status_response(U2F_COMMAND_ERROR, CTAP1_ERR_CHANNEL_BUSY);
        } else if (transport_type == TRANSPORT_BLE) {
            fido_ble_send_status_response(U2F_COMMAND_ERROR, CTAP1_ERR_CHANNEL_BUSY);
        }
        return true;
    }
    return false;
}

//
// USB HID/BLE/NFCの各トランスポートにより、
// リクエストデータの全フレーム受信が完了した時の処理
//
static void on_hid_request_receive_completed(void)
{
    // データ受信後に実行すべき処理を判定
    uint8_t cmd = fido_hid_receive_header()->CMD;
    if (is_waiting_user_presence_verify(TRANSPORT_HID, cmd)) {
        // ユーザー所在確認中はエラーを戻す
        return;
    }
    switch (cmd) {
#if CTAP2_SUPPORTED
        case CTAP2_COMMAND_INIT:
            fido_ctap2_command_hid_init();
            break;
        case CTAP2_COMMAND_PING:
            fido_u2f_command_ping(TRANSPORT_HID);
            break;
        case CTAP2_COMMAND_WINK:
            fido_ctap2_command_wink();
            break;
        case CTAP2_COMMAND_LOCK:
            fido_ctap2_command_lock();
            break;
        case CTAP2_COMMAND_CANCEL:
            // キャンセルコマンドの場合は
            // ユーザー所在確認待ちをキャンセルしたうえで
            // キャンセルレスポンスを戻す
            fido_ctap2_command_cancel();
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
        case MNT_COMMAND_INSTALL_ATTESTATION:
        case MNT_COMMAND_RESET_ATTESTATION:
            fido_development_command(TRANSPORT_HID);
            break;
        case (0x80 | MNT_COMMAND_BASE):
            fido_maintenance_command(TRANSPORT_HID);
            break;
        default:
            // 不正なコマンドであるため
            // エラーレスポンスを送信
            fido_log_error("Invalid command (0x%02x) ", cmd);
            fido_hid_send_status_response(U2F_COMMAND_ERROR, CTAP1_ERR_INVALID_COMMAND);
            break;
    }
}

static void on_ble_request_receive_completed(void)
{
    BLE_HEADER_T *p_ble_header = fido_ble_receive_header();

    // データ受信後に実行すべき処理を判定
    uint8_t       cmd = p_ble_header->CMD;
    if (is_waiting_user_presence_verify(TRANSPORT_BLE, cmd)) {
        // ユーザー所在確認中はエラーを戻す
        return;
    }
    switch (cmd) {
        case U2F_COMMAND_MSG:
            if (fido_ble_receive_ctap2_command() != 0x00) {
                if (fido_ble_receive_ctap2_command() < MNT_COMMAND_BASE) {
                    // CTAP2コマンドを処理する。
                    fido_ctap2_command_cbor(TRANSPORT_BLE);
                } else {
                    // 管理用コマンドを処理する。
                    fido_maintenance_command(TRANSPORT_BLE);
                }
            } else {
                // U2Fコマンドを処理する。
                fido_u2f_command_msg(TRANSPORT_BLE);
            }
            break;
        case CTAP2_COMMAND_PING:
            // PINGレスポンスを実行
            fido_u2f_command_ping(TRANSPORT_BLE);
            break;
        case U2F_COMMAND_CANCEL:
            // TODO: 後日実装
            break;
        default:
            break;
    }
}

void fido_command_on_request_receive_completed(TRANSPORT_TYPE transport_type)
{
    if (abort_flag) {
        // 全業務閉塞中の場合はここで終了
        return;
    }

    switch (transport_type) {
        case TRANSPORT_HID:
            on_hid_request_receive_completed();
            break;
        case TRANSPORT_BLE:
            on_ble_request_receive_completed();
            break;
        case TRANSPORT_NFC:
            // 現在のところ、CTAP2コマンドのみをサポート
            fido_ctap2_command_cbor(transport_type);
            break;
        default:
            break;
    }
}

//
// USB HID/BLE/NFCの各トランスポートにより、
// レスポンスデータの全フレーム送信が完了した時の処理
//
static void on_ble_response_send_completed(void)
{
    // 受信フレーム数カウンターをクリア
    fido_ble_receive_frame_count_clear();

    // 全フレーム送信後に行われる後続処理を実行
    uint8_t cmd = fido_ble_receive_header()->CMD;
    switch (cmd) {
        case U2F_COMMAND_MSG:
            if (fido_ble_receive_ctap2_command() != 0x00) {
                if (fido_ble_receive_ctap2_command() < MNT_COMMAND_BASE) {
                    // CTAP2コマンドを処理する。
                    fido_ctap2_command_cbor_response_sent();
                } else {
                    // 管理用コマンドを処理する。
                    fido_maintenance_command_report_sent();
                }
            } else {
                // U2Fコマンドを処理する。
                fido_u2f_command_msg_response_sent();
            }
            break;
        case CTAP2_COMMAND_PING:
            fido_u2f_command_ping_response_sent();
            break;
        case U2F_COMMAND_CANCEL:
            // TODO: 後日実装
            break;
        default:
            break;
    }
}

void on_hid_response_send_completed(void)
{
    // 全フレーム送信後に行われる後続処理を実行
    uint8_t cmd = fido_hid_receive_header()->CMD;
    switch (cmd) {
        case CTAP2_COMMAND_INIT:
            fido_ctap2_command_init_response_sent();
            break;
        case CTAP2_COMMAND_PING:
            fido_u2f_command_ping_response_sent();
            break;
        case U2F_COMMAND_MSG:
            fido_u2f_command_msg_response_sent();
            break;
        case CTAP2_COMMAND_CBOR:
            fido_ctap2_command_cbor_response_sent();
            break;
        case MNT_COMMAND_INSTALL_ATTESTATION:
        case MNT_COMMAND_RESET_ATTESTATION:
            fido_development_command_report_sent();
            break;
        case (0x80 | MNT_COMMAND_BASE):
            fido_maintenance_command_report_sent();
            break;
        default:
            break;
    }
}

void fido_command_on_response_send_completed(TRANSPORT_TYPE transport_type)
{
    switch (transport_type) {
        case TRANSPORT_HID:
            on_hid_response_send_completed();
            break;
        case TRANSPORT_BLE:
            on_ble_response_send_completed();
            break;
        case TRANSPORT_NFC:
            // 現在のところ、CTAP2コマンドのみをサポート
            fido_ctap2_command_cbor_response_sent();
            break;
        default:
            break;
    }

    if (abort_flag) {
        // レスポンス完了後の処理を停止させる場合は、
        // 全色LEDを点灯させたのち、全業務を閉塞
        fido_status_indicator_abort();
    }
}

//
// FIDO BLE関連
//
bool fido_command_is_valid_ble_command(uint8_t command)
{
    // FIDO BLEの仕様で定義されている
    // 受信可能コマンドである場合、true を戻す
    switch (command) {
        case U2F_COMMAND_PING:
        case U2F_COMMAND_MSG:
        case U2F_COMMAND_CANCEL:
            return true;
        default:
            return false;
    }
}
