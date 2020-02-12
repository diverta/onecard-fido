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
#include "fido_hid_receive.h"
#include "fido_hid_send.h"
#include "fido_maintenance.h"
#include "ctap2_common.h"
#include "u2f.h"

// デモ機能（BLEデバイスによる自動認証機能）
#include "demo_ble_peripheral_auth.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// ユーザー所在確認タイムアウト（３０秒）
#define USER_PRESENCE_VERIFY_TIMEOUT_MSEC 30000

// ユーザー所在確認待ち状態を示すフラグ
static bool waiting_for_tup = false;

// レスポンス完了後の処理を停止させるフラグ
static bool abort_flag = false;

bool fido_command_do_abort(void)
{
    // レスポンス完了後の処理を停止させる場合は、
    // 全色LEDを点灯させたのち、無限ループに入る
    if (abort_flag) {
        fido_status_indicator_abort();
        while(true);
    }
    return abort_flag;
}

void fido_command_abort_flag_set(bool flag)
{
    abort_flag = flag;
}

//
// CTAP2、U2Fで共用する各種処理
//
void fido_command_mainsw_event_handler(void)
{
    // ボタンが短押しされた時の処理を実行
    if (fido_u2f_command_on_mainsw_event() == true) {
        return;
    }
    fido_ctap2_command_on_mainsw_event();
}

void fido_command_keepalive_timer_handler(void)
{
    // キープアライブ・コマンドを実行する
    fido_ctap2_command_keepalive_timer_handler();
    fido_u2f_command_keepalive_timer_handler();
}

void fido_user_presence_verify_timeout_handler(void) 
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
    fido_user_presence_verify_timer_start(USER_PRESENCE_VERIFY_TIMEOUT_MSEC, NULL);

    // ユーザー所在確認待ち状態に入る
    waiting_for_tup = true;

    // 自動認証機能が有効な場合は
    // ボタンを押す代わりに
    // 指定のサービスUUIDをもつBLEペリフェラルをスキャン
    if (demo_ble_peripheral_auth_start_scan()) {
        return;
    }

    // 赤色LED高速点滅開始
    fido_status_indicator_prompt_reset();
}

void fido_user_presence_verify_start(uint32_t timeout_msec)
{
    // キープアライブタイマーを開始
    fido_repeat_process_timer_start(timeout_msec, fido_command_keepalive_timer_handler);

    // ユーザー所在確認タイムアウト監視を開始
    fido_user_presence_verify_timer_start(USER_PRESENCE_VERIFY_TIMEOUT_MSEC, NULL);

    // ユーザー所在確認待ち状態に入る
    waiting_for_tup = true;

    // 自動認証機能が有効な場合は
    // ボタンを押す代わりに
    // 指定のサービスUUIDをもつBLEペリフェラルをスキャン
    if (demo_ble_peripheral_auth_start_scan()) {
        return;
    }

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

void fido_user_presence_verify_on_ble_scan_end(bool success)
{
    // 自動認証機能が有効時、指定のサービスUUIDをもつ
    // BLEペリフェラルデバイスのスキャン完了時の処理
    if (success) {
        // スキャン成功時は、ボタン押下時と等価の処理を実行
        fido_command_mainsw_event_handler();
    } else {
        // スキャン失敗時は、タイムアウト時と等価の処理を実行
        fido_user_presence_verify_timeout_handler();
        // タイムアウトの旨のステータスを戻す
        //   BLEデバイススキャン自体が、HID経由で呼び出される処理なので、
        //   HIDチャネルにレスポンスを送信
        fido_hid_send_status_response(U2F_COMMAND_ERROR, CTAP1_ERR_TIMEOUT);
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
        case MNT_COMMAND_ERASE_SKEY_CERT:
        case MNT_COMMAND_INSTALL_SKEY_CERT:
        case MNT_COMMAND_GET_FLASH_STAT:
        case MNT_COMMAND_GET_APP_VERSION:
        case MNT_COMMAND_PREFERENCE_PARAM:
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

static void on_ble_request_receive_completed(void)
{
    BLE_HEADER_T *p_ble_header = fido_ble_receive_header();
    FIDO_APDU_T  *p_apdu = fido_ble_receive_apdu();

    // データ受信後に実行すべき処理を判定
    uint8_t       cmd = p_ble_header->CMD;
    if (is_waiting_user_presence_verify(TRANSPORT_BLE, cmd)) {
        // ユーザー所在確認中はエラーを戻す
        return;
    }
    if (cmd == U2F_COMMAND_MSG) {
        if (p_apdu->CLA != 0x00) {
            // CTAP2コマンドを処理する。
            fido_ctap2_command_cbor(TRANSPORT_BLE);

        } else {
            // U2Fコマンド／管理用コマンドを処理する。
            fido_u2f_command_msg(TRANSPORT_BLE);
        }

    } else if (cmd == U2F_COMMAND_PING) {
        // PINGレスポンスを実行
        fido_u2f_command_ping(TRANSPORT_BLE);
    }
}

void fido_command_on_request_receive_completed(TRANSPORT_TYPE transport_type)
{
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
    if (fido_ble_receive_header()->CMD == U2F_COMMAND_MSG) {
        if (fido_ble_receive_apdu()->CLA != 0x00) {
            // CTAP2コマンドを処理する。
            fido_ctap2_command_cbor_response_sent();

        } else {
            // U2Fコマンド／管理用コマンドを処理する。
            fido_u2f_command_msg_response_sent();
        }
    }
    if (fido_ble_receive_header()->CMD == U2F_COMMAND_PING) {
        fido_u2f_command_ping_response_sent();
    }
}

void on_hid_response_send_completed(void)
{
    // 全フレーム送信後に行われる後続処理を実行
    uint8_t cmd = fido_hid_receive_header()->CMD;
    switch (cmd) {
        case CTAP2_COMMAND_INIT:
            fido_log_info("CTAPHID_INIT end");
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
        case MNT_COMMAND_ERASE_SKEY_CERT:
        case MNT_COMMAND_INSTALL_SKEY_CERT:
        case MNT_COMMAND_GET_FLASH_STAT:
        case MNT_COMMAND_GET_APP_VERSION:
        case MNT_COMMAND_PREFERENCE_PARAM:
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

    if (fido_command_do_abort()) {
        // レスポンス完了後の処理を停止させる場合はここで終了
        return;
    }
}
