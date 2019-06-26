#include "sdk_common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ble_u2f.h"
#include "ble_u2f_command.h"
#include "ble_u2f_control_point.h"
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

// Flash ROM更新が完了時、
// 後続処理が使用するデータを共有
static ble_u2f_context_t m_u2f_context;

ble_u2f_context_t *get_ble_u2f_context(void)
{
    return &m_u2f_context;
}

bool ble_u2f_command_on_mainsw_event(ble_u2f_t *p_u2f)
{
    if (p_u2f->conn_handle == BLE_CONN_HANDLE_INVALID) {
        // U2Fクライアントと接続中でないときに
        // MAIN SWが押下された場合は無視
        return false;
    }

    if (m_u2f_context.p_ble_header == NULL) {
        // BLEリクエスト受信(ble_u2f_control_point_receive)より
        // 前の時点の場合は無視
        return false;
    }

    // CMD,INS,P1を参照
    uint8_t cmd = m_u2f_context.p_ble_header->CMD;
    uint8_t ins = m_u2f_context.p_apdu->INS;
    uint8_t control_byte = m_u2f_context.p_apdu->P1;
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
    // 共有情報をゼロクリアする
    memset(&m_u2f_context, 0, sizeof(ble_u2f_context_t));
    fido_log_debug("ble_u2f_command_initialize_context done ");
    
    // コマンド／リクエストデータ格納領域を初期化する
    ble_u2f_control_point_initialize();
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
    BLE_HEADER_T *p_ble_header = m_u2f_context.p_ble_header;
    FIDO_APDU_T *p_apdu = m_u2f_context.p_apdu;

    // 受信したリクエストデータが、
    // 初期導入データか、リクエストデータの
    // どちらかであるのか判定する
    if (p_ble_header->CMD == U2F_COMMAND_MSG) {
        if (p_apdu->Lc == p_apdu->data_length) {
            if (p_ble_header->CONT == true) {
                // 後続パケットが存在するのでスルー
                return COMMAND_NONE;
            }

            // 受信データをすべて受け取った場合は以下の処理
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

            if (m_u2f_context.p_apdu->CLA != 0x00) {
                // INSが正しくてもCLAが不正の場合は
                // エラーレスポンスを送信して終了
                fido_log_debug("get_command_type: Invalid CLA(0x%02x) ", m_u2f_context.p_apdu->CLA);
                ble_u2f_send_error_response(p_ble_header->CMD, U2F_SW_CLA_NOT_SUPPORTED);
                return COMMAND_NONE;
            }

            uint16_t status_word = m_u2f_context.p_ble_header->STATUS_WORD;
            if (status_word != 0x00) {
                // リクエストの検査中にステータスワードが設定された場合は
                // エラーレスポンスを送信して終了
                ble_u2f_send_error_response(p_ble_header->CMD, status_word);
                return COMMAND_NONE;
            }
            
        } else {
            // データが完成していないのでスルー
            current_command = COMMAND_NONE;
        }

    } else if (p_ble_header->CMD == U2F_COMMAND_PING) {
        if (p_apdu->Lc == p_apdu->data_length) {
            // データが完成していれば、PINGレスポンスを実行
            fido_log_debug("get_command_type: PING Request Message received ");
            current_command = COMMAND_U2F_PING;
        } else {
            // データが完成していないのでスルー
            current_command = COMMAND_NONE;
        }

    } else if (p_ble_header->CMD == U2F_COMMAND_ERROR) {
        // リクエストデータの検査中にエラーが確認された場合、
        // エラーレスポンスを戻す
        m_u2f_context.p_ble_header = p_ble_header;
        ble_u2f_send_command_error_response(p_ble_header->ERROR);

        // 以降のリクエストデータは読み捨てる
        p_ble_header->CMD = 0;
        current_command = COMMAND_NONE;

    } else {
        current_command = COMMAND_NONE;
    }

    return current_command;
}

void ble_u2f_command_on_ble_evt_write(ble_u2f_t *p_u2f, ble_gatts_evt_write_t *p_evt_write)
{
    // コマンドバッファに入力されたリクエストデータを取得
    ble_u2f_control_point_receive(p_evt_write);

    // データ受信後に実行すべき処理を判定
    fido_ble_receive_command_set(get_command_type());
    
    // ペアリングモード時はペアリング以外の機能を実行できないようにするため
    // エラーステータスワード (0x9601) を戻す
    if (fido_ble_pairing_mode_get() == true && fido_ble_receive_command_get() != COMMAND_PAIRING) {
        ble_u2f_send_error_response(m_u2f_context.p_ble_header->CMD, 0x9601);
        return;
    }

    switch (fido_ble_receive_command_get()) {
        case COMMAND_INITBOND:
            fido_ble_pairing_delete_bonds();
            break;
            
        case COMMAND_PAIRING:
            ble_u2f_send_success_response(m_u2f_context.p_ble_header->CMD);
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
    if (fido_ble_receive_command_get() == COMMAND_CHANGE_PAIRING_MODE) {
        fido_ble_pairing_reflect_mode_change(p_evt);
        return;
    }
        
    // コマンドが確定していない場合は終了
    if (fido_ble_receive_command_get() == COMMAND_NONE) {
        return;
    }

    // CTAP2コマンドの処理を実行
    ble_ctap2_command_on_fs_evt(p_evt);
    
    // Flash ROM更新後に行われる後続処理を実行
    switch (fido_ble_receive_command_get()) {
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
    ble_u2f_control_point_receive_frame_count_clear();

    // 次回リクエストまでの経過秒数監視をスタート
    fido_comm_interval_timer_start();

    // CTAP2コマンドの処理を実行
    ble_ctap2_command_response_sent();
}
