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
#include "fido_hid_receive.h"
#include "fido_hid_send.h"
#include "fido_maintenance.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// レスポンスデータ格納領域
//
static uint8_t response_buffer[1024];

// ランダムベクター生成領域
static uint8_t m_random_vector[32];

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
    // 秘密鍵／証明書をFlash ROM領域から削除
    // (fds_file_deleteが実行される)
    fido_log_info("Erase private key and certificate start");
    if (fido_flash_skey_cert_delete() == false) {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 1);
    }
}

static bool generate_random_password(void)
{
    // 32バイトのランダムベクターを生成
    fido_crypto_generate_random_vector(m_random_vector, sizeof(m_random_vector));

    // Flash ROMに書き出して保存
    if (fido_flash_password_set(m_random_vector) == false) {
        return false;
    }

    fido_log_debug("Generated random vector for AES password ");
    return true;
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

    // Flash ROMに登録する鍵・証明書データを準備
    if (fido_flash_skey_cert_data_prepare(data, length) == false) {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 8);
        return;
    }

    // 証明書データをFlash ROMへ書込
    // (fds_record_update/writeまたはfds_gcが実行される)
    if (fido_flash_skey_cert_write() == false) {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 9);
    }
}

static void command_get_flash_stat(void)
{
    fido_log_info("Get flash ROM statistics start");

    // 統計情報CSVを取得
    size_t response_size = sizeof(response_buffer);
    if (fido_flash_get_stat_csv(response_buffer, &response_size) == false) {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 10);
    }

    // レスポンスを送信
    //  データ形式
    //  0-3: CID（0xffffffff）
    //  4:   CMD（0xc2）
    //  5-6: データサイズ（CSVデータの長さ）
    //  7:   ステータスバイト（成功時は 0x00）
    //  8-n: CSVデータ（下記のようなCSV形式のテキスト）
    //       <項目名1>=<値2>,<項目名2>=<値2>,...,<項目名k>=<値k>
    send_command_response(CTAP1_ERR_SUCCESS, response_size);
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
        case MNT_COMMAND_GET_FLASH_STAT:
            command_get_flash_stat();
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
        case MNT_COMMAND_GET_FLASH_STAT:
            fido_log_info("Get flash ROM statistics end");
            break;
        default:
            break;
    }
}

void fido_maintenance_command_flash_failed(void)
{
    // Flash ROM処理でエラーが発生時はエラーレスポンス送信
    uint8_t cmd = fido_hid_receive_header()->CMD;
    switch (cmd) {
        case MNT_COMMAND_ERASE_SKEY_CERT:
            send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 3);
            fido_log_error("Erase private key and certificate abend");
            break;
        case MNT_COMMAND_INSTALL_SKEY_CERT:
            send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 9);
            fido_log_error("Install private key and certificate abend");
            break;
        default:
            break;
    }
}

void fido_maintenance_command_flash_gc_done(void)
{
    // for nRF52840:
    // FDSリソース不足解消のためGCが実行された場合は、
    // GC実行直前の処理を再実行
    uint8_t cmd = fido_hid_receive_header()->CMD;
    switch (cmd) {
        case MNT_COMMAND_ERASE_SKEY_CERT:
            if (generate_random_password() == false) {
                send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 5);
            }
            break;
        case MNT_COMMAND_INSTALL_SKEY_CERT:
            fido_log_warning("Install private key and certificate retry: FDS GC done ");
            command_install_skey_cert();
            break;
        default:
            break;
    }
}

void fido_maintenance_command_skey_cert_file_deleted(void)
{
    if (fido_hid_receive_header()->CMD == MNT_COMMAND_ERASE_SKEY_CERT) {
        // 秘密鍵／証明書削除が完了
        fido_log_debug("Erase private key and certificate file completed ");

        // 続いて、トークンカウンターをFlash ROM領域から削除
        // (fds_file_deleteが実行される)
        if (fido_flash_token_counter_delete() == false) {
            send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 2);
        }
    }
}

void fido_maintenance_command_token_counter_file_deleted(void)
{
    if (fido_hid_receive_header()->CMD == MNT_COMMAND_ERASE_SKEY_CERT) {
        // トークンカウンター削除が完了
        fido_log_debug("Erase token counter file completed");

        // 続いて、AES秘密鍵生成処理を行う
        // (fds_record_update/writeまたはfds_gcが実行される)
        if (generate_random_password() == false) {
            send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 4);
        }
    }
}

void fido_maintenance_command_aes_password_record_updated(void)
{
    if (fido_hid_receive_header()->CMD == MNT_COMMAND_ERASE_SKEY_CERT) {
        // AES秘密鍵生成(fds_record_update/write)完了
        fido_log_debug("Update AES password record completed ");

        // レスポンスを生成してU2Fクライアントに戻す
        send_command_error_response(CTAP1_ERR_SUCCESS);
    }
}

void fido_maintenance_command_skey_cert_record_updated(void)
{
    if (fido_hid_receive_header()->CMD == MNT_COMMAND_INSTALL_SKEY_CERT) {
        // 証明書データ書込完了
        fido_log_debug("Update private key and certificate record completed ");

        // レスポンスを生成してU2Fクライアントに戻す
        send_command_error_response(CTAP1_ERR_SUCCESS);
    }
}
