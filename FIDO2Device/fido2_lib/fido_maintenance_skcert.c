/* 
 * File:   fido_maintenance_skcert.c
 * Author: makmorit
 *
 * Created on 2020/02/06, 10:41
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//
// プラットフォーム非依存コード
//
#include "fido_hid_receive.h"
#include "fido_hid_send.h"
#include "fido_command_common.h"
#include "fido_maintenance.h"
#include "fido_maintenance_cryption.h"
#include "atecc.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// for debug apdu data
#define LOG_DEBUG_PUBKEY_BUFF   false

// データ編集用エリア（領域節約のため共通化）
static uint8_t work_buf[64];

static void send_command_response(uint8_t ctap2_status, size_t length)
{
    // レスポンスデータを送信パケットに設定し送信
    uint32_t cid = fido_hid_receive_header()->CID;
    uint32_t cmd = fido_hid_receive_header()->CMD;
    // １バイトめにステータスコードをセット
    work_buf[0] = ctap2_status;
    fido_hid_send_command_response(cid, cmd, work_buf, length);
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
    fido_command_generate_random_vector(work_buf, 32);

    // Flash ROMに書き出して保存
    if (fido_flash_password_set(work_buf) == false) {
        return false;
    }

    fido_log_debug("Generated random vector for AES password ");
    return true;
}

static bool install_privkey_to_atecc(void)
{
    // 秘密鍵をATECC608Aの１４番スロットに登録
    if (atecc_install_privkey(fido_maintenance_cryption_data()) == false) {
        return false;
    }

    // 証明書から公開鍵を抽出
    // （証明書は公開鍵の後ろに連結されている）
    uint8_t *cert_data = fido_maintenance_cryption_data() + RAW_PRIVATE_KEY_SIZE;
    size_t   cert_size = fido_maintenance_cryption_size() - RAW_PRIVATE_KEY_SIZE;
    uint8_t *pubkey = fido_extract_pubkey_in_certificate(cert_data, cert_size);
    if (pubkey == NULL) {
        fido_log_error("install_privkey_to_atecc failed: No public key in certificate");
        return false;
    } 

    // １４番スロットの秘密鍵から、公開鍵を生成
    if (atecc_generate_pubkey_from_privkey(work_buf) == false) {
        return false;
    }

#if LOG_DEBUG_PUBKEY_BUFF
    // 生成された公開鍵をダンプ
    fido_log_debug("Public key from certificate:");
    fido_log_print_hexdump_debug(pubkey, RAW_PUBLIC_KEY_SIZE);
    fido_log_debug("Public key from ATECC608A:");
    fido_log_print_hexdump_debug(work_buf, RAW_PUBLIC_KEY_SIZE);
#endif

    // 内容を検証
    if (memcmp(pubkey, work_buf, RAW_PUBLIC_KEY_SIZE) != 0) {
        fido_log_error("install_privkey_to_atecc failed: Invalid public key in certificate");
        return false;
    }

    // クライアントから送付されたデータから、
    // 秘密鍵に該当する部分を削除
    // （先頭の32バイトをFlash ROM上での初期状態とする）
    memset(fido_maintenance_cryption_data(), 0xff, RAW_PRIVATE_KEY_SIZE);
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

    // リクエストデータをCBORデコードし、鍵・証明書データを復号化
    uint8_t *cbor_data_buffer = data + 1;
    size_t   cbor_data_length = length - 1;
    if (fido_maintenance_cryption_restore(cbor_data_buffer, cbor_data_length) == false) {
        return;
    }
    
    // Flash ROMに登録済みのデータがあれば領域に読込
    if (fido_flash_skey_cert_read() == false) {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 7);
        return;
    }

    // ATECC608Aが利用可能であれば、
    // 秘密鍵をFlash ROMに登録せず、
    // ATECC608Aの該当スロットに登録するようにする
    if (atecc_is_available()) {
        if (install_privkey_to_atecc() == false) {
            send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 7);
            return;
        }
    }
    
    // Flash ROMに登録する鍵・証明書データを準備
    if (fido_flash_skey_cert_data_prepare(
        fido_maintenance_cryption_data(), fido_maintenance_cryption_size()) == false) {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 8);
        return;
    }

    // 証明書データをFlash ROMへ書込
    // (fds_record_update/writeまたはfds_gcが実行される)
    if (fido_flash_skey_cert_write() == false) {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 9);
    }
}

void fido_maintenance_command_skey_cert(void)
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

void fido_maintenance_command_skey_cert_report_sent(void)
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

        // 続いて、署名カウンター情報をFlash ROM領域から削除
        // (fds_file_deleteが実行される)
        if (fido_command_sign_counter_delete() == false) {
            send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 2);
        }
    }
}

void fido_maintenance_command_token_counter_file_deleted(void)
{
    if (fido_hid_receive_header()->CMD == MNT_COMMAND_ERASE_SKEY_CERT) {
        // 署名カウンター情報削除が完了
        fido_log_debug("Erase token counter file completed");

        // 続いて、AESパスワード生成処理を行う
        // (fds_record_update/writeまたはfds_gcが実行される)
        if (generate_random_password() == false) {
            send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 4);
        }
    }
}

void fido_maintenance_command_aes_password_record_updated(void)
{
    if (fido_hid_receive_header()->CMD == MNT_COMMAND_ERASE_SKEY_CERT ||
        fido_hid_receive_header()->CMD == MNT_COMMAND_INSTALL_SKEY_CERT) {
        // AESパスワード生成(fds_record_update/write)完了
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

        // AESパスワードが生成済みの場合は終了
        if (fido_command_check_aes_password_exist()) {
            fido_log_debug("AES password record already exists ");
            send_command_error_response(CTAP1_ERR_SUCCESS);
            return;
        }

        // 続いて、AESパスワード生成処理を行う
        // (fds_record_update/writeまたはfds_gcが実行される)
        if (generate_random_password() == false) {
            send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 4);
        }
    }
}
