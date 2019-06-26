#include "sdk_common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ble_u2f.h"
#include "ble_u2f_command.h"
#include "ble_u2f_status.h"
#include "ble_u2f_register.h"
#include "ble_u2f_authenticate.h"
#include "ble_u2f_version.h"
#include "fido_ble_receive.h"
#include "fido_timer.h"

// for CTAP2 support
#include "ctap2_common.h"
#include "ble_ctap2_command.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// for fido_ble_peripheral_mode
#include "fido_ble_peripheral.h"

// for BLE pairing functions
#include "fido_ble_pairing.h"

// 初期設定コマンド群(鍵・証明書の新規導入用等)
#define U2F_INS_INSTALL_INITBOND 0x41
#define U2F_INS_INSTALL_INITFSTR 0x42
#define U2F_INS_INSTALL_INITSKEY 0x43
#define U2F_INS_INSTALL_INITCERT 0x44
#define U2F_INS_INSTALL_PAIRING  0x45

//
// ble_u2f_commandで判定されたコマンドを保持
//
static enum COMMAND_TYPE command;

void fido_ble_command_set(enum COMMAND_TYPE c)
{
    command = c;
}

bool ble_u2f_command_on_mainsw_event(ble_u2f_t *p_u2f)
{
    if (p_u2f->conn_handle == BLE_CONN_HANDLE_INVALID) {
        // U2Fクライアントと接続中でないときに
        // MAIN SWが押下された場合は無視
        return false;
    }

    if (fido_ble_receive_header() == NULL) {
        // BLEリクエスト受信(ble_u2f_control_point_receive)より
        // 前の時点の場合は無視
        return false;
    }

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

bool ble_u2f_command_on_mainsw_long_push_event(ble_u2f_t *p_u2f)
{
    UNUSED_PARAMETER(p_u2f);
    if (!fido_ble_peripheral_mode()) {
        // BLEペリフェラルが稼働していないときに
        // MAIN SWが長押しされた場合は無視
        return false;
    }

    // ペアリングモード変更を実行
    fido_ble_pairing_change_mode();
    return true;
}

void ble_u2f_command_initialize_context(void)
{
    // コマンド／リクエストデータ格納領域を初期化する
    fido_ble_receive_init();
    fido_log_debug("ble_u2f_command_initialize_context done ");
}


void ble_u2f_command_finalize_context(void)
{
    // ユーザー所在確認を停止(キープアライブを停止)
    fido_user_presence_terminate();
    fido_log_debug("ble_u2f_command_finalize_context done ");
}

static enum COMMAND_TYPE get_command_type(void)
{
    enum COMMAND_TYPE current_command;

    // BLEヘッダー、APDUの参照を取得
    BLE_HEADER_T *p_ble_header = fido_ble_receive_header();
    FIDO_APDU_T  *p_apdu = fido_ble_receive_apdu();

    // 受信したリクエストデータが、
    // 初期導入データか、リクエストデータの
    // どちらかであるのか判定する
    if (p_ble_header->CMD == U2F_COMMAND_MSG) {
        // CTAP2コマンドから先に処理する。
        if (is_ctap2_command_byte(p_apdu->CLA)) {
            return COMMAND_CTAP2_COMMAND;
        }

        // 以下はU2Fコマンド
        if (p_apdu->INS == U2F_REGISTER) {
            fido_log_debug("get_command_type: Registration Request Message received ");
            current_command = COMMAND_U2F_REGISTER;

        } else if (p_apdu->INS == U2F_AUTHENTICATE) {
            fido_log_debug("get_command_type: Authentication Request Message received ");
            current_command = COMMAND_U2F_AUTHENTICATE;

        } else if (p_apdu->INS == U2F_VERSION) {
            fido_log_debug("get_command_type: GetVersion Request Message received ");
            current_command = COMMAND_U2F_VERSION;

        // 初期導入関連コマンドの場合
        // (vendor defined command)
        } else if (p_apdu->INS == U2F_INS_INSTALL_INITBOND) {
            // ボンディング情報削除コマンド
            fido_log_debug("get_command_type: initialize bonding information ");
            current_command = COMMAND_INITBOND;

        } else if (p_apdu->INS == U2F_INS_INSTALL_PAIRING) {
            // ペアリングのためのレスポンスを実行
            fido_log_debug("get_command_type: pairing request received ");
            current_command = COMMAND_PAIRING;

        } else {
            // INSが不正の場合は終了
            fido_log_debug("get_command_type: Invalid INS(0x%02x) ", p_apdu->INS);
            ble_u2f_send_error_response(p_ble_header->CMD, U2F_SW_INS_NOT_SUPPORTED);
            return COMMAND_NONE;
        }

        if (fido_ble_receive_apdu()->CLA != 0x00) {
            // INSが正しくてもCLAが不正の場合は
            // エラーレスポンスを送信して終了
            fido_log_debug("get_command_type: Invalid CLA(0x%02x) ", fido_ble_receive_apdu()->CLA);
            ble_u2f_send_error_response(p_ble_header->CMD, U2F_SW_CLA_NOT_SUPPORTED);
            return COMMAND_NONE;
        }

        uint16_t status_word = fido_ble_receive_header()->STATUS_WORD;
        if (status_word != 0x00) {
            // リクエストの検査中にステータスワードが設定された場合は
            // エラーレスポンスを送信して終了
            ble_u2f_send_error_response(p_ble_header->CMD, status_word);
            return COMMAND_NONE;
        }

    } else if (p_ble_header->CMD == U2F_COMMAND_PING) {
        // データが完成していれば、PINGレスポンスを実行
        fido_log_debug("get_command_type: PING Request Message received ");
        current_command = COMMAND_U2F_PING;

    } else if (p_ble_header->CMD == U2F_COMMAND_ERROR) {
        // リクエストデータの検査中にエラーが確認された場合、
        // エラーレスポンスを戻す
        ble_u2f_send_command_error_response(p_ble_header->ERROR);

        // 以降のリクエストデータは読み捨てる
        p_ble_header->CMD = 0;
        current_command = COMMAND_NONE;

    } else {
        current_command = COMMAND_NONE;
    }

    return current_command;
}

void fido_ble_command_on_request_received(void)
{
    // データ受信後に実行すべき処理を判定
    fido_ble_command_set(get_command_type());
    
    // ペアリングモード時はペアリング以外の機能を実行できないようにするため
    // エラーステータスワード (0x9601) を戻す
    if (fido_ble_pairing_mode_get() == true && command != COMMAND_PAIRING) {
        ble_u2f_send_error_response(fido_ble_receive_header()->CMD, 0x9601);
        return;
    }

    switch (command) {
        case COMMAND_INITBOND:
            fido_ble_pairing_delete_bonds();
            break;
            
        case COMMAND_PAIRING:
            ble_u2f_send_success_response(fido_ble_receive_header()->CMD);
            break;

        case COMMAND_CTAP2_COMMAND:
            ble_ctap2_command_do_process();
            break;

        case COMMAND_U2F_REGISTER:
            ble_u2f_register_do_process();
            break;

        case COMMAND_U2F_AUTHENTICATE:
            ble_u2f_authenticate_do_process();
            break;

        case COMMAND_U2F_VERSION:
            ble_u2f_version_do_process();
            break;

        case COMMAND_U2F_PING:
            ble_u2f_status_response_ping();
            break;

        default:
            break;
    }
}


void ble_u2f_command_on_fs_evt(fds_evt_t const *const p_evt)
{
    // ペアリングモード変更時のイベントを優先させる
    if (command == COMMAND_CHANGE_PAIRING_MODE) {
        fido_ble_pairing_reflect_mode_change(p_evt);
        return;
    }
        
    // コマンドが確定していない場合は終了
    if (command == COMMAND_NONE) {
        return;
    }

    // CTAP2コマンドの処理を実行
    ble_ctap2_command_on_fs_evt(p_evt);
    
    // Flash ROM更新後に行われる後続処理を実行
    switch (command) {
        case COMMAND_U2F_REGISTER:
            ble_u2f_register_send_response(p_evt);
            break;

        case COMMAND_U2F_AUTHENTICATE:
            ble_u2f_authenticate_send_response(p_evt);
            break;

        default:
            break;
    }
}

void ble_u2f_command_keepalive_timer_handler(void *p_context)
{
    // キープアライブ・コマンドを実行する
    uint8_t *p_keepalive_status_byte = (uint8_t *)p_context;
    ble_u2f_send_keepalive_response(*p_keepalive_status_byte);
}

void ble_u2f_command_on_response_send_completed(void)
{
    // 受信フレーム数カウンターをクリア
    fido_ble_receive_frame_count_clear();

    // 次回リクエストまでの経過秒数監視をスタート
    fido_comm_interval_timer_start();

    // CTAP2コマンドの処理を実行
    ble_ctap2_command_response_sent();
}
