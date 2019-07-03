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
#include "fido_u2f_command.h"    // for U2F support

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

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

    if (p_ble_header->CMD == U2F_COMMAND_MSG) {
        if (p_apdu->CLA != 0x00) {
            // CTAP2コマンドを処理する。
            fido_ctap2_command_cbor(TRANSPORT_BLE);

        } else {
            // U2Fコマンド／管理用コマンドを処理する。
            fido_u2f_command_msg(TRANSPORT_BLE);
        }

    } else if (p_ble_header->CMD == U2F_COMMAND_PING) {
        // PINGレスポンスを実行
        send_ping_response();
    }
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
