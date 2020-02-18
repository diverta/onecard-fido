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
#include "fido_maintenance_skcert.h"

// for ATECC608A
#include "fido_cryptoauth.h"
#include "fido_cryptoauth_aes_cbc.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// for debug hex dump data
#define LOG_HEXDUMP_DEBUG_PUBKEY false

//
// レスポンスデータ格納領域
//
static uint8_t response_buffer[4];

// ランダムベクター生成領域
static uint8_t m_random_vector[32];

// 証明書検証用の公開鍵
static uint8_t public_key_ref[RAW_PUBLIC_KEY_SIZE];

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
    // 証明書をFlash ROM領域から削除
    // (fds_file_deleteが実行される)
    fido_log_info("Erase certificate start");
    if (fido_flash_skey_cert_delete() == false) {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 1);
    }
}

static bool verify_cert(uint8_t *cert_data, size_t cert_size)
{
    // 外部証明書から公開鍵を抽出
    if (fido_cryptoauth_extract_pubkey_from_cert(public_key_ref, cert_data, cert_size) == false) {
        return false;
    }

#if LOG_HEXDUMP_DEBUG_PUBKEY
    fido_log_debug("Public key of certificate:");
    fido_log_print_hexdump_debug(public_key_ref, RAW_PUBLIC_KEY_SIZE);
#endif

    // １４番スロットの秘密鍵から、公開鍵を生成
    uint8_t *p_public_key = fido_cryptoauth_keypair_public_key(KEY_ID_FOR_INSTALL_PRIVATE_KEY);

#if LOG_HEXDUMP_DEBUG_PUBKEY
    // 生成された公開鍵をダンプ
    fido_log_debug("Public key from ATECC608A:");
    fido_log_print_hexdump_debug(p_public_key, RAW_PUBLIC_KEY_SIZE);
#endif

    // 内容を検証
    if (memcmp(public_key_ref, p_public_key, sizeof(public_key_ref)) != 0) {
        fido_log_error("verify_cert failed: Invalid public key");
        return false;
    }

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

    // 秘密鍵をATECC608Aに保管
    if (fido_cryptoauth_install_privkey(fido_maintenance_cryption_data()) == false) {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 7);
        return;
    }

    // 証明書を検証
    uint8_t *cert_data = fido_maintenance_cryption_data() + RAW_PRIVATE_KEY_SIZE;
    size_t   cert_size = fido_maintenance_cryption_size() - RAW_PRIVATE_KEY_SIZE;
    if (verify_cert(cert_data, cert_size) == false) {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 7);
        return;
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
            fido_log_info("Erase certificate end");
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
            fido_log_error("Erase certificate abend");
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
            fido_log_warning("Erase certificate retry: FDS GC done ");
            command_erase_skey_cert();
            break;
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
        // 証明書削除が完了
        fido_log_debug("Erase certificate file completed ");

        // 続いて、トークンカウンターをFlash ROM領域から削除
        // (fds_file_deleteが実行される)
        if (fido_command_sign_counter_delete() == false) {
            send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 2);
        }
    }
}

void fido_maintenance_command_token_counter_file_deleted(void)
{
    if (fido_hid_receive_header()->CMD == MNT_COMMAND_ERASE_SKEY_CERT) {
        // トークンカウンター削除が完了
        fido_log_debug("Erase token counter file completed");

        // 続いて、AESパスワード消去処理を行う
        if (fido_cryptoauth_aes_cbc_erase_password() == false) {
            send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 4);
            return;
        }
        // AESパスワード生成完了
        fido_log_debug("Erase AES password completed ");
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
            fido_log_debug("AES password already exists ");
            send_command_error_response(CTAP1_ERR_SUCCESS);
            return;
        }

        // 続いて、AESパスワード生成処理を行う
        if (fido_cryptoauth_aes_cbc_new_password() == false) {
            send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 4);
            return;
        }
        // AESパスワード生成完了
        fido_log_debug("Generate new AES password completed ");
        send_command_error_response(CTAP1_ERR_SUCCESS);
    }
}
