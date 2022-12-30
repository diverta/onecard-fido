/* 
 * File:   fido_maintenance.c
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
#include "fido_ble_receive.h"
#include "fido_ble_send.h"
#include "fido_hid_receive.h"
#include "fido_hid_send.h"
#include "fido_maintenance.h"
#include "fido_maintenance_define.h"
#include "u2f.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(fido_maintenance);
#endif

// トランスポート種別を保持
static TRANSPORT_TYPE m_transport_type;

static uint8_t get_maintenance_command_byte(void)
{
    //
    // 管理用コマンドバイトを、データ部の先頭から抽出
    //
    uint8_t cmd = 0x00;
    switch (m_transport_type) {
        case TRANSPORT_HID:
            if (fido_hid_receive_header()->CMD == (0x80 | MNT_COMMAND_BASE)) {
                cmd = fido_hid_receive_apdu()->data[0];
            }
            break;
        case TRANSPORT_BLE:
            if (fido_ble_receive_header()->CMD == U2F_COMMAND_MSG) {
                cmd = fido_ble_receive_apdu()->data[0];
            }
            break;
        default:
            break;
    }
    return cmd;
}

static uint8_t *get_maintenance_data_buffer(void)
{
    uint8_t *buffer;
    switch (m_transport_type) {
        case TRANSPORT_HID:
            buffer = fido_hid_receive_apdu()->data + 1;
            break;
        case TRANSPORT_BLE:
            buffer = fido_ble_receive_apdu()->data + 1;
            break;
        default:
            buffer = NULL;
            break;
    }
    return buffer;
}

static size_t get_maintenance_data_buffer_size(void)
{
    size_t size;
    switch (m_transport_type) {
        case TRANSPORT_HID:
            size = fido_hid_receive_apdu()->Lc - 1;
            break;
        case TRANSPORT_BLE:
            size = fido_ble_receive_apdu()->Lc - 1;
            break;
        default:
            size = 0;
            break;
    }
    return size;
}

// 関数プロトタイプ
static void command_erase_bonding_data_response(bool success);

//
// レスポンスデータ格納領域
//
static uint8_t response_buffer[1024];

static void send_command_response(uint8_t ctap2_status, size_t length)
{
    // １バイトめにステータスコードをセット
    response_buffer[0] = ctap2_status;

    // レスポンスデータを送信パケットに設定し送信
    if (m_transport_type == TRANSPORT_HID) {
        uint32_t cid = fido_hid_receive_header()->CID;
        uint8_t cmd = fido_hid_receive_header()->CMD;
        fido_hid_send_command_response(cid, cmd, response_buffer, length);

    } else if (m_transport_type == TRANSPORT_BLE) {
        uint8_t cmd = fido_ble_receive_header()->CMD;
        fido_ble_send_command_response(cmd, response_buffer, length);
    } 
}

static void send_command_error_response(uint8_t ctap2_status) 
{
    // レスポンスデータを送信パケットに設定し送信
    //   エラーなので送信バイト数＝１
    send_command_response(ctap2_status, 1);
}

static void command_get_flash_stat(void)
{
    fido_log_info("Get flash ROM statistics start");
    //
    // 統計情報CSVを取得
    //  response_bufferの先頭にステータスバイトを格納するため、
    //  CSV格納領域は、response_bufferの２バイト目を先頭とします。
    //
    uint8_t *buffer = response_buffer + 1;
    size_t   buffer_size = sizeof(response_buffer - 1);
    if (fido_flash_get_stat_csv(buffer, &buffer_size) == false) {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 10);
        return;
    }
    //
    // レスポンスを送信
    //  データ形式
    //  0-3: CID（0xffffffff）
    //  4:   CMD（0xc2）
    //  5-6: データサイズ（CSVデータの長さ）
    //  7:   ステータスバイト（成功時は 0x00）
    //  8-n: CSVデータ（下記のようなCSV形式のテキスト）
    //       <項目名1>=<値2>,<項目名2>=<値2>,...,<項目名k>=<値k>
    //
    send_command_response(CTAP1_ERR_SUCCESS, buffer_size + 1);
}

static void command_get_app_version(void)
{
    fido_log_info("Get application version info start");
    //
    // バージョン情報CSVを取得
    //  response_bufferの先頭にステータスバイトを格納するため、
    //  CSV格納領域は、response_bufferの２バイト目を先頭とします。
    //
    uint8_t *buffer = response_buffer + 1;
    size_t   buffer_size = sizeof(response_buffer - 1);
    if (fido_board_get_version_info_csv(buffer, &buffer_size) == false) {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 10);
        return;
    }
    //
    // レスポンスを送信
    //  データ形式
    //  0-3: CID（0xffffffff）
    //  4:   CMD（0xc3）
    //  5-6: データサイズ（CSVデータの長さ）
    //  7:   ステータスバイト（成功時は 0x00）
    //  8-n: CSVデータ（下記のようなCSV形式のテキスト）
    //       <項目名1>=<値2>,<項目名2>=<値2>,...,<項目名k>=<値k>
    //
    send_command_response(CTAP1_ERR_SUCCESS, buffer_size + 1);
}

static void command_bootloader_mode(void)
{
    // 最初にレスポンスを送信
    if (usbd_service_support_bootloader_mode()) {
        send_command_response(CTAP1_ERR_SUCCESS, 1);
    } else {
        fido_log_error("Bootloader mode is not supported");
        send_command_response(CTAP1_ERR_INVALID_COMMAND, 1);
    }
}

static void jump_to_bootloader_mode(void)
{
    // ブートローダーモードに遷移させるための処理を実行
    if (usbd_service_support_bootloader_mode()) {
        usbd_service_stop_for_bootloader();
    }
}

static void command_pairing_request(void)
{
    // レスポンスを送信
    send_command_response(CTAP1_ERR_SUCCESS, 1);
}

static void command_unpairing_request(void)
{
    // リクエスト格納領域
    uint8_t *request_data = get_maintenance_data_buffer();
    size_t   request_size = get_maintenance_data_buffer_size();

    // レスポンス格納領域
    uint8_t *response_data = response_buffer + 1;
    size_t   response_size = sizeof(response_buffer - 1);

    // ペアリング解除要求コマンドを実行し、レスポンスを生成
    if (fido_ble_unpairing_request(request_data, request_size, response_data, &response_size) == false) {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST);
        return;
    }

    // レスポンスを送信
    send_command_response(CTAP1_ERR_SUCCESS, response_size + 1);
}

static void command_unpairing_cancel(void)
{
    // ペアリング解除要求キャンセルコマンドを実行
    fido_ble_unpairing_cancel_request();

    // レスポンスを送信
    send_command_response(CTAP1_ERR_SUCCESS, 1);
}

static void command_erase_bonding_data(void)
{
    fido_log_info("Erase bonding data start");
    //
    // nRF52840のFlash ROM上に作成された
    // 全てのペアリング情報を削除
    //
    if (fido_ble_unpairing_erase_bond_data(command_erase_bonding_data_response) == false) {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 12);
    }
}

static void command_erase_bonding_data_response(bool success)
{
    // レスポンスを送信
    if (success) {
        send_command_response(CTAP1_ERR_SUCCESS, 1);
    } else {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 13);
    }
}

static void command_system_reset(void)
{
    // 最初にレスポンスを送信
    fido_log_info("System reset will start immediately...");
    send_command_response(CTAP1_ERR_SUCCESS, 1);
}

static void command_get_timestamp(void)
{
    // レスポンスを送信（20バイト固定長）
    // 0: ステータス
    // 1: "yyyy/mm/dd hh:mm:ss"形式の文字列
    size_t length = 20;
    if (rtcc_get_timestamp_string((char *)response_buffer + 1, length)) {
        send_command_response(CTAP1_ERR_SUCCESS, length);

    } else {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST);
    }
}

static void command_set_timestamp(void)
{
    uint8_t *data = get_maintenance_data_buffer();
    uint16_t length = get_maintenance_data_buffer_size();

    // 元データチェック
    if (data == NULL || length != 4) {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST);
        return;
    }

    // 現在時刻を設定
    // リクエスト＝４バイトのUNIX時間整数（ビッグエンディアン）
    uint32_t seconds_since_epoch = fido_get_uint32_from_bytes(data);
    uint8_t timezone_diff_hours = 9;
    if (rtcc_update_timestamp_by_unixtime(seconds_since_epoch, timezone_diff_hours) == false) {
        send_command_response(CTAP2_ERR_VENDOR_FIRST, 1);
        return;
    }

    // レスポンスとして、現在時刻を送信
    command_get_timestamp();
}

void fido_maintenance_command(TRANSPORT_TYPE transport_type)
{
    // トランスポート種別を保持
    m_transport_type = transport_type;

    // リクエストデータ受信後に実行すべき処理を判定
    uint8_t mnt_cmd = get_maintenance_command_byte();
    switch (mnt_cmd) {
        case MNT_COMMAND_PAIRING_REQUEST:
            command_pairing_request();
            return;
        case MNT_COMMAND_UNPAIRING_REQUEST:
            command_unpairing_request();
            return;
        case MNT_COMMAND_UNPAIRING_CANCEL:
            command_unpairing_cancel();
            return;
        case MNT_COMMAND_ERASE_BONDING_DATA:
            command_erase_bonding_data();
            return;
        case MNT_COMMAND_BOOTLOADER_MODE:
            command_bootloader_mode();
            return;
        case MNT_COMMAND_SYSTEM_RESET:
            command_system_reset();
            return;
        case MNT_COMMAND_GET_FLASH_STAT:
            command_get_flash_stat();
            return;
        case MNT_COMMAND_GET_APP_VERSION:
            command_get_app_version();
            return;
        case MNT_COMMAND_GET_TIMESTAMP:
            command_get_timestamp();
            return;
        case MNT_COMMAND_SET_TIMESTAMP:
            command_set_timestamp();
            return;
        default:
            break;
    }
}

void fido_maintenance_command_report_sent(void)
{
    // 全フレーム送信後に行われる後続処理を実行
    uint8_t mnt_cmd = get_maintenance_command_byte();
    switch (mnt_cmd) {
        case MNT_COMMAND_ERASE_BONDING_DATA:
            fido_log_info("Erase bonding data end");
            return;
        case MNT_COMMAND_BOOTLOADER_MODE:
            jump_to_bootloader_mode();
            return;
        case MNT_COMMAND_SYSTEM_RESET:
            // nRF52840のシステムリセットを実行
            fido_board_system_reset();
            return;
        case MNT_COMMAND_GET_FLASH_STAT:
            fido_log_info("Get flash ROM statistics end");
            return;
        case MNT_COMMAND_GET_APP_VERSION:
            fido_log_info("Get application version info end");
            return;
        case MNT_COMMAND_GET_TIMESTAMP:
        case MNT_COMMAND_SET_TIMESTAMP:
            return;
        default:
            break;
    }
}

void fido_maintenance_command_flash_failed(void)
{
    // Flash ROM処理でエラーが発生時はエラーレスポンス送信
    uint8_t mnt_cmd = get_maintenance_command_byte();
    switch (mnt_cmd) {
        default:
            break;
    }
}

void fido_maintenance_command_flash_gc_done(void)
{
    // for nRF52840:
    // FDSリソース不足解消のためGCが実行された場合は、
    // GC実行直前の処理を再実行
    uint8_t mnt_cmd = get_maintenance_command_byte();
    switch (mnt_cmd) {
        default:
            break;
    }
}
