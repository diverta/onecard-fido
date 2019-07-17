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
#include "fido_command.h"
#include "fido_ctap2_command.h"  // for CTAP2 support
#include "fido_u2f_command.h"    // for U2F support

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

void fido_ble_command_send_status_response(uint8_t cmd, uint8_t status_code) 
{
    // U2F ERRORコマンドに対応する
    // レスポンスデータを送信パケットに設定し送信
    fido_ble_send_command_response_no_callback(cmd, status_code);
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
    if (invalid_command_in_pairing_mode(p_ble_header->CMD, p_apdu->INS)) {
        fido_ble_send_status_word(p_ble_header->CMD, 0x9601);
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
        fido_u2f_command_ping(TRANSPORT_BLE);
    }
}
