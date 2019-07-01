/* 
 * File:   fido_ble_command.c
 * Author: makmorit
 *
 * Created on 2019/06/26, 16:12
 */
//
// プラットフォーム非依存コード
//
#include "u2f.h"
#include "ctap2_common.h"
#include "fido_ble_receive.h"
#include "fido_ble_receive_apdu.h"
#include "fido_ble_send.h"
#include "fido_ctap2_command.h"  // for CTAP2 support

// 移行中のモジュール
#include "fido_ble_pairing.h"
#include "ble_u2f_register.h"
#include "ble_u2f_authenticate.h"
#include "ble_u2f_version.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// 初期設定コマンド群(鍵・証明書の新規導入用等)
#define U2F_INS_INSTALL_INITBOND 0x41
#define U2F_INS_INSTALL_PAIRING  0x45

// ペアリングモード変更中の旨を保持
static bool change_pairing_mode = false;

bool fido_ble_command_on_mainsw_event(void)
{
    // CMD,INS,P1を参照
    uint8_t cmd = fido_ble_receive_header()->CMD;
    uint8_t ins = fido_ble_receive_apdu()->INS;
    uint8_t control_byte = fido_ble_receive_apdu()->P1;
    if (cmd == 0x83 && ins == 0x02 && control_byte == 0x03) {
        // 0x83 ("MSG") 
        // 0x02 ("U2F_AUTHENTICATE")
        // 0x03 ("enforce-user-presence-and-sign") 
        // ユーザー所在確認が取れたと判定し、
        // キープアライブを停止
        fido_log_info("ble_u2f_authenticate: completed the test of user presence");
        fido_user_presence_verify_end();

        // Authenticationの後続処理を実行する
        ble_u2f_authenticate_resume_process();
        return true;

    } else {
        // 他のコマンドの場合は無視
        return false;
    }
}

void fido_ble_command_send_status_response(uint8_t cmd, uint8_t status_code) 
{
    // U2F ERRORコマンドに対応する
    // レスポンスデータを送信パケットに設定し送信
    fido_ble_send_command_response_no_callback(cmd, status_code);

    // アイドル時点滅処理を開始
    fido_idling_led_on();
}

void fido_ble_command_send_status_word(uint8_t command_for_response, uint16_t err_status_word)
{    
    // ステータスワードを格納
    uint8_t cmd_response_buffer[2];
    fido_set_status_word(cmd_response_buffer, err_status_word);
    
    // レスポンスを送信
    fido_ble_send_command_response(command_for_response, cmd_response_buffer, sizeof(cmd_response_buffer));
}

static void send_ping_response(void)
{
    // BLE接続情報、BLEヘッダー、APDUの参照を取得
    BLE_HEADER_T *p_ble_header = fido_ble_receive_header();
    FIDO_APDU_T *p_apdu = fido_ble_receive_apdu();

    // PINGの場合は
    // リクエストのBLEヘッダーとデータを編集せず
    // レスポンスとして戻す（エコーバック）
    fido_ble_send_command_response(p_ble_header->CMD, p_apdu->data, p_apdu->data_length);
}

static bool ctap2_command_do_process(uint8_t command_byte)
{
    // 受信データの先頭１バイトめで処理を判定
    switch (command_byte) {
        case CTAP2_CMD_GETINFO:
        case CTAP2_CMD_RESET:
        case CTAP2_CMD_MAKE_CREDENTIAL:
        case CTAP2_CMD_GET_ASSERTION:
            fido_ctap2_command_cbor(TRANSPORT_BLE);
            return true;
        default:
            return false;
    }
}

void fido_ble_command_on_request_received(void)
{
    // BLEヘッダー、APDUの参照を取得
    BLE_HEADER_T *p_ble_header = fido_ble_receive_header();
    FIDO_APDU_T  *p_apdu = fido_ble_receive_apdu();

    if (p_ble_header->CMD == U2F_COMMAND_ERROR) {
        // リクエストデータの検査中にエラーが確認された場合、
        // エラーレスポンスを戻す
        fido_ble_command_send_status_response(U2F_COMMAND_ERROR, fido_ble_receive_header()->ERROR);
        return;
    }
    
    // ペアリングモード時はペアリング以外の機能を実行できないようにするため
    // エラーステータスワード (0x9601) を戻す
    if (fido_ble_pairing_mode_get()) {
        if (p_ble_header->CMD == U2F_COMMAND_MSG &&
            p_apdu->INS == U2F_INS_INSTALL_PAIRING) {
            fido_ble_command_send_status_word(fido_ble_receive_header()->CMD, U2F_SW_NO_ERROR);
        } else {
            fido_ble_command_send_status_word(fido_ble_receive_header()->CMD, 0x9601);
        }
        return;
    }

    // CTAP2コマンドを処理する。
    if (ctap2_command_do_process(p_apdu->CLA)) {
        return;
    }

    if (p_ble_header->CMD == U2F_COMMAND_MSG) {
        // U2Fコマンド／管理用コマンドを処理する。
        switch (p_apdu->INS) {
            case U2F_INS_INSTALL_INITBOND:
                fido_ble_pairing_delete_bonds();
                break;
            case U2F_REGISTER:
                ble_u2f_register_do_process();
                break;
            case U2F_AUTHENTICATE:
                ble_u2f_authenticate_do_process();
                break;
            case U2F_VERSION:
                ble_u2f_version_do_process();
                break;
            default:
                // INSが不正の場合は終了
                fido_log_debug("get_command_type: Invalid INS(0x%02x) ", p_apdu->INS);
                fido_ble_command_send_status_word(p_ble_header->CMD, U2F_SW_INS_NOT_SUPPORTED);
                break;
        }
    }

    if (p_ble_header->CMD == U2F_COMMAND_PING) {
        // PINGレスポンスを実行
        send_ping_response();
        return;
    }
}

void fido_ble_command_set_change_pairing_mode(void)
{
    change_pairing_mode = true;
}

void fido_ble_command_on_fs_evt(void const *p_evt)
{
    // ペアリングモード変更時のイベントを優先させる
    if (change_pairing_mode) {
        fido_ble_pairing_reflect_mode_change(p_evt);
        return;
    }

    // CTAP2コマンドの処理を実行
    fido_ctap2_command_cbor_send_response(p_evt);
    
    // Flash ROM更新後に行われる後続処理を実行
    BLE_HEADER_T *p_ble_header = fido_ble_receive_header();
    FIDO_APDU_T  *p_apdu = fido_ble_receive_apdu();
    if (p_ble_header->CMD == U2F_COMMAND_MSG) {
        // U2Fコマンド／管理用コマンドを処理する。
        switch (p_apdu->INS) {
            case U2F_REGISTER:
                ble_u2f_register_send_response(p_evt);
                break;
            case U2F_AUTHENTICATE:
                ble_u2f_authenticate_send_response(p_evt);
                break;
            default:
                break;
        }
    }
}

void fido_ble_command_keepalive_timer_handler(void *p_context)
{
    // キープアライブ・コマンドを実行する
    uint8_t *p_keepalive_status_byte = (uint8_t *)p_context;
    fido_ble_send_command_response_no_callback(U2F_COMMAND_KEEPALIVE, *p_keepalive_status_byte);
}

void fido_ble_command_on_response_send_completed(void)
{
    // 受信フレーム数カウンターをクリア
    fido_ble_receive_frame_count_clear();

    // 次回リクエストまでの経過秒数監視をスタート
    fido_comm_interval_timer_start();

    // CTAP2コマンドの処理を実行
    fido_ctap2_command_cbor_response_completed();
}
