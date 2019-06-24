/* 
 * File:   fido_maintenance.h
 * Author: makmorit
 *
 * Created on 2019/03/26, 13:35
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//
// プラットフォーム非依存コード
//
#include "fido_maintenance.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// プラットフォーム依存コード
// ターゲットごとの実装となります。
//
#include "fido_hid_receive.h"
#include "fido_hid_send.h"
#include "fido_flash.h"
#include "fido_flash_password.h"

// 秘密鍵／証明書削除が完了したかどうかを保持
static bool skey_cert_deleted = false;

//
// レスポンスデータ格納領域
//
static uint8_t response_buffer[1024];

static void send_command_response(uint8_t ctap2_status, size_t length)
{
    // レスポンスデータを送信パケットに設定し送信
    uint32_t cid = fido_hid_receive_header()->CID;
    uint32_t cmd = fido_hid_receive_header()->CMD;
    // １バイトめにステータスコードをセット
    response_buffer[0] = ctap2_status;
    fido_hid_send_command_response(cid, cmd, response_buffer, length);
}

static void send_command_error_response(uint8_t ctap2_status) 
{
    // レスポンスデータを送信パケットに設定し送信
    //   エラーなので送信バイト数＝１
    send_command_response(ctap2_status, 1);
}

static void command_erase_skey_cert(void)
{
    // 秘密鍵／証明書削除が完了した旨のフラグをクリア
    skey_cert_deleted = false;

    // 秘密鍵／証明書をFlash ROM領域から削除
    // (fds_file_deleteが実行される)
    fido_log_info("Erase private key and certificate start");
    if (fido_flash_skey_cert_delete() == false) {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 1);
        return;
    }
    // トークンカウンターをFlash ROM領域から削除
    // (fds_file_deleteが実行される)
    if (fido_flash_token_counter_delete() == false) {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 2);
        return;
    }
}

static void command_erase_skey_cert_response(fido_flash_event_t const *const p_evt)
{
    if (p_evt->result == false) {
        // エラーレスポンスを生成してU2Fクライアントに戻す
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 3);
        fido_log_error("Erase private key and certificate abend");
        return;
    }

    if (p_evt->delete_file) {
        if (skey_cert_deleted == false) {
            // 秘密鍵／証明書削除が完了した旨のフラグを設定し、
            // 次のイベント発生を待つ
            skey_cert_deleted = true;
            fido_log_debug("fido_flash_skey_cert_delete completed ");

        } else {
            // トークンカウンター削除が完了
            fido_log_debug("fido_flash_token_counter_delete completed ");

            // 続いて、AES秘密鍵生成処理を行う
            // (fds_record_update/writeまたはfds_gcが実行される)
            if (fido_flash_password_generate() == false) {
                send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 4);
            }
        }

    } else if (p_evt->gc) {
        // FDSリソース不足解消のためGCが実行された場合は、
        // GC実行直前の処理を再実行
        if (fido_flash_password_generate() == false) {
            send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 5);
        }
        
    } else if (p_evt->write_update) {
        // AES秘密鍵生成(fds_record_update/write)完了の場合
        // レスポンスを生成してU2Fクライアントに戻す
        send_command_error_response(CTAP1_ERR_SUCCESS);
    }
}

static void command_install_skey_cert(void)
{
    uint8_t *data = fido_hid_receive_apdu()->data;
    uint16_t length = fido_hid_receive_apdu()->Lc;

    // 元データチェック
    if (data == NULL || length == 0) {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 6);
        return;
    }
    fido_log_info("Install private key and certificate start");

    // Flash ROMに登録済みのデータがあれば領域に読込
    if (fido_flash_skey_cert_read() == false) {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 7);
        return;
    }
    uint32_t *securekey_buffer = fido_flash_skey_cert_data();

    // 秘密鍵部（リクエストデータの先頭32バイト）を領域に格納
    memcpy(securekey_buffer, data, 32);

    // 証明書データ格納領域の開始アドレスを取得
    uint32_t *cert_buffer = securekey_buffer + SKEY_WORD_NUM;
    uint16_t  cert_length = length - 32;

    // 証明書データの格納に必要なワード数をチェックする
    uint32_t cert_buffer_length = (cert_length - 1) / 4 + 2;
    if (cert_buffer_length > CERT_WORD_NUM) {
        fido_log_error("cert data words(%d) exceeds max words(%d) ",
            cert_buffer_length, CERT_WORD_NUM);
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 8);
        return;
    }
    
    // １ワード目に、証明書の当初バイト数を格納し、
    // ２ワード目以降から、証明書のデータを格納するようにする
    // (エンディアンは変換せずにそのまま格納)
    cert_buffer[0] = (uint32_t)cert_length;
    memcpy(cert_buffer + 1, data + 32, cert_length);

    // 証明書データをFlash ROMへ書込
    // (fds_record_update/writeまたはfds_gcが実行される)
    if (fido_flash_skey_cert_write() == false) {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 9);
    }
}

static void command_install_skey_cert_response(fido_flash_event_t const *const p_evt)
{
    if (p_evt->result == false) {
        // FDS処理でエラーが発生時は以降の処理を行わない
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 9);
        fido_log_error("Install private key and certificate abend");
        return;
    }

    if (p_evt->gc) {
        // FDSリソース不足解消のためGCが実行された場合は、
        // GC実行直前の処理を再実行
        fido_log_warning("Install private key and certificate retry: FDS GC done ");
        command_install_skey_cert();

    } else if (p_evt->write_update) {
        // レスポンスを生成してU2Fクライアントに戻す
        send_command_error_response(CTAP1_ERR_SUCCESS);
    }
}

void fido_maintenance_command(void)
{
    // リクエストデータ受信後に実行すべき処理を判定
    uint8_t cmd = fido_hid_receive_header()->CMD;
    switch (cmd) {
        case MNT_COMMAND_ERASE_SKEY_CERT:
            command_erase_skey_cert();
            break;
        case MNT_COMMAND_INSTALL_SKEY_CERT:
            command_install_skey_cert();
            break;
        default:
            break;
    }
}

void fido_maintenance_command_send_response(fido_flash_event_t const *const p_evt)
{
    // Flash ROM更新完了時の処理を実行
    uint8_t cmd = fido_hid_receive_header()->CMD;
    switch (cmd) {
        case MNT_COMMAND_ERASE_SKEY_CERT:
            command_erase_skey_cert_response(p_evt);
            break;
        case MNT_COMMAND_INSTALL_SKEY_CERT:
            command_install_skey_cert_response(p_evt);
            break;
        default:
            break;
    }
}

void fido_maintenance_command_report_sent(void)
{
    // 全フレーム送信後に行われる後続処理を実行
    uint8_t cmd = fido_hid_receive_header()->CMD;
    switch (cmd) {
        case MNT_COMMAND_ERASE_SKEY_CERT:
            fido_log_info("Erase private key and certificate end");
            break;
        case MNT_COMMAND_INSTALL_SKEY_CERT:
            fido_log_info("Install private key and certificate end");
            break;
        default:
            break;
    }
}
