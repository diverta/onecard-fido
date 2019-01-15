#include "sdk_common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ble_u2f.h"
#include "ble_u2f_control_point.h"
#include "ble_u2f_status.h"
#include "ble_u2f_register.h"
#include "ble_u2f_authenticate.h"
#include "ble_u2f_version.h"
#include "ble_u2f_securekey.h"
#include "ble_u2f_pairing.h"
#include "ble_u2f_flash.h"
#include "ble_u2f_util.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ble_u2f_command
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for user presence test
#include "fido_user_presence.h"

// Flash ROM更新が完了時、
// 後続処理が使用するデータを共有
static ble_u2f_context_t m_u2f_context;


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
        NRF_LOG_INFO("ble_u2f_authenticate: completed the test of user presence");
        m_u2f_context.user_presence_byte = fido_user_presence_verify_end();

        // Authenticationの後続処理を実行する
        ble_u2f_authenticate_resume_process(&m_u2f_context);
        return true;

    } else {
        // 他のコマンドの場合は無視
        return false;
    }
}

bool ble_u2f_command_on_mainsw_long_push_event(ble_u2f_t *p_u2f)
{
    // ペアリングモード変更を実行
    ble_u2f_pairing_change_mode(&m_u2f_context);
    return true;
}

void ble_u2f_command_initialize_context(void)
{
    // 共有情報をゼロクリアする
    memset(&m_u2f_context, 0, sizeof(ble_u2f_context_t));
    NRF_LOG_DEBUG("ble_u2f_command_initialize_context done ");
    
    // コマンド／リクエストデータ格納領域を初期化する
    ble_u2f_control_point_initialize();
}


void ble_u2f_command_finalize_context(void)
{
    // ユーザー所在確認を停止(キープアライブを停止)
    fido_user_presence_terminate();
    NRF_LOG_DEBUG("ble_u2f_command_finalize_context done ");
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
            if (p_apdu->INS == U2F_REGISTER) {
                NRF_LOG_DEBUG("get_command_type: Registration Request Message received ");
                current_command = COMMAND_U2F_REGISTER;

            } else if (p_apdu->INS == U2F_AUTHENTICATE) {
                NRF_LOG_DEBUG("get_command_type: Authentication Request Message received ");
                current_command = COMMAND_U2F_AUTHENTICATE;

            } else if (p_apdu->INS == U2F_VERSION) {
                NRF_LOG_DEBUG("get_command_type: GetVersion Request Message received ");
                current_command = COMMAND_U2F_VERSION;

            // 初期導入関連コマンドの場合
            // (vendor defined command)
            } else if (p_apdu->INS == U2F_INS_INSTALL_INITBOND) {
                // ボンディング情報削除コマンド
                NRF_LOG_DEBUG("get_command_type: initialize bonding information ");
                current_command = COMMAND_INITBOND;

            } else if (p_apdu->INS == U2F_INS_INSTALL_INITFSTR) {
                // 秘密鍵／証明書削除コマンド
                NRF_LOG_DEBUG("get_command_type: initialize fstorage ");
                current_command = COMMAND_INITFSTR;

            } else if (p_apdu->INS == U2F_INS_INSTALL_INITSKEY) {
                // 秘密鍵導入コマンド
                NRF_LOG_DEBUG("get_command_type: initial skey data received ");
                current_command = COMMAND_INITSKEY;

            } else if (p_apdu->INS == U2F_INS_INSTALL_INITCERT) {
                // データが完成していれば、証明書導入用コマンドを実行
                NRF_LOG_DEBUG("get_command_type: initial cert data received ");
                current_command = COMMAND_INITCERT;
                
            } else if (p_apdu->INS == U2F_INS_INSTALL_PAIRING) {
                // ペアリングのためのレスポンスを実行
                NRF_LOG_DEBUG("get_command_type: pairing request received ");
                current_command = COMMAND_PAIRING;

            } else {
                // INSが不正の場合は終了
                NRF_LOG_DEBUG("get_command_type: Invalid INS(0x%02x) ", p_apdu->INS);
                ble_u2f_send_error_response(&m_u2f_context, U2F_SW_INS_NOT_SUPPORTED);
                return COMMAND_NONE;
            }

            if (m_u2f_context.p_apdu->CLA != 0x00) {
                // INSが正しくてもCLAが不正の場合は
                // エラーレスポンスを送信して終了
                NRF_LOG_DEBUG("get_command_type: Invalid CLA(0x%02x) ", m_u2f_context.p_apdu->CLA);
                ble_u2f_send_error_response(&m_u2f_context, U2F_SW_CLA_NOT_SUPPORTED);
                return COMMAND_NONE;
            }

            uint16_t status_word = m_u2f_context.p_ble_header->STATUS_WORD;
            if (status_word != 0x00) {
                // リクエストの検査中にステータスワードが設定された場合は
                // エラーレスポンスを送信して終了
                ble_u2f_send_error_response(&m_u2f_context, status_word);
                return COMMAND_NONE;
            }
            
        } else {
            // データが完成していないのでスルー
            current_command = COMMAND_NONE;
        }

    } else if (p_ble_header->CMD == U2F_COMMAND_PING) {
        if (p_apdu->Lc == p_apdu->data_length) {
            // データが完成していれば、PINGレスポンスを実行
            NRF_LOG_DEBUG("get_command_type: PING Request Message received ");
            current_command = COMMAND_U2F_PING;
        } else {
            // データが完成していないのでスルー
            current_command = COMMAND_NONE;
        }

    } else if (p_ble_header->CMD == U2F_COMMAND_ERROR) {
        // リクエストデータの検査中にエラーが確認された場合、
        // エラーレスポンスを戻す
        m_u2f_context.p_ble_header = p_ble_header;
        ble_u2f_send_command_error_response(&m_u2f_context, p_ble_header->ERROR);

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
    // BLE接続情報を共有情報に保持
    m_u2f_context.p_u2f = p_u2f;

    // コマンドバッファに入力されたリクエストデータを取得
    ble_u2f_control_point_receive(p_evt_write, &m_u2f_context);

    // データ受信後に実行すべき処理を判定
    m_u2f_context.command = get_command_type();
    
    // ペアリングモード時はペアリング以外の機能を実行できないようにするため
    // エラーステータスワード (0x9601) を戻す
    if (ble_u2f_pairing_mode_get() == true && m_u2f_context.command != COMMAND_PAIRING) {
        ble_u2f_send_error_response(&m_u2f_context, 0x9601);
        return;
    }

    switch (m_u2f_context.command) {
        case COMMAND_INITBOND:
            ble_u2f_pairing_delete_bonds(&m_u2f_context);
            break;

        case COMMAND_INITFSTR:
            ble_u2f_securekey_erase(&m_u2f_context);
            break;

        case COMMAND_INITSKEY:
            ble_u2f_securekey_install_skey(&m_u2f_context);
            break;

        case COMMAND_INITCERT:
            ble_u2f_securekey_install_cert(&m_u2f_context);
            break;
            
        case COMMAND_PAIRING:
            ble_u2f_send_success_response(&m_u2f_context);
            break;

        case COMMAND_U2F_REGISTER:
            ble_u2f_register_do_process(&m_u2f_context);
            break;

        case COMMAND_U2F_AUTHENTICATE:
            ble_u2f_authenticate_do_process(&m_u2f_context);
            break;

        case COMMAND_U2F_VERSION:
            ble_u2f_version_do_process(&m_u2f_context);
            break;

        case COMMAND_U2F_PING:
            ble_u2f_status_response_ping(&m_u2f_context);
            break;

        default:
            break;
    }
}


void ble_u2f_command_on_fs_evt(fds_evt_t const *const p_evt)
{
    // ペアリングモード変更時のイベントを優先させる
    if (m_u2f_context.command == COMMAND_CHANGE_PAIRING_MODE) {
        ble_u2f_pairing_reflect_mode_change(&m_u2f_context, p_evt);
        return;
    }

    // 共有情報の中に接続情報がない場合は終了
    ble_u2f_t *p_u2f = m_u2f_context.p_u2f;
    if (p_u2f == NULL) {
        return;
    }
    
    // Flash ROM更新後に行われる後続処理を実行
    switch (m_u2f_context.command) {
        case COMMAND_INITFSTR:
            ble_u2f_securekey_erase_response(&m_u2f_context, p_evt);
            break;

        case COMMAND_INITSKEY:
            ble_u2f_securekey_install_skey_response(&m_u2f_context, p_evt);
            break;

        case COMMAND_INITCERT:
            ble_u2f_securekey_install_cert_response(&m_u2f_context, p_evt);
            break;

        case COMMAND_U2F_REGISTER:
            ble_u2f_register_send_response(&m_u2f_context, p_evt);
            break;

        case COMMAND_U2F_AUTHENTICATE:
            ble_u2f_authenticate_send_response(&m_u2f_context, p_evt);
            break;

        default:
            break;
    }
}

void ble_u2f_command_keepalive_timer_handler(void *p_context)
{
    // キープアライブ・コマンドを実行する
    ble_u2f_context_t *p_u2f_context = (ble_u2f_context_t *)p_context;
    ble_u2f_send_keepalive_response(p_u2f_context);
}
