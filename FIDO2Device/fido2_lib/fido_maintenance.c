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
#include "fido_ble_receive.h"
#include "fido_ble_send.h"
#include "fido_hid_receive.h"
#include "fido_hid_send.h"
#include "fido_maintenance.h"
#include "fido_maintenance_skcert.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(fido_maintenance);
#endif

// トランスポート種別を保持
static TRANSPORT_TYPE m_transport_type;

static uint8_t get_command_byte(void)
{
    uint8_t cmd;
    switch (m_transport_type) {
        case TRANSPORT_HID:
            cmd = fido_hid_receive_header()->CMD;
            break;
        case TRANSPORT_BLE:
            cmd = fido_ble_receive_header()->CMD;
            break;
        default:
            cmd = 0x00;
            break;
    }
    return cmd;
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

static void command_preference_parameter_maintenance(void)
{
    uint8_t *data = fido_hid_receive_apdu()->data;
    uint16_t length = fido_hid_receive_apdu()->Lc;

    // 元データチェック
    if (data == NULL || length == 0) {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 6);
        return;
    }
    fido_log_info("Preference parameter maintenance start");

    // データの１バイト目からコマンド種別を取得
    uint8_t cmd_type = data[0];

    //
    // 各種設定用パラメーター管理
    //  response_bufferの先頭にステータスバイトを格納するため、
    //  レスポンス格納領域は、response_bufferの２バイト目を先頭とします。
    //
    uint8_t *buffer = response_buffer + 1;
    size_t   buffer_size = sizeof(response_buffer - 1);
    bool     ret = false;
    switch (cmd_type) {
        case 1:
        case 2:
        case 3:
            ble_peripheral_auth_param_request(data, length);
            ret = ble_peripheral_auth_param_response(cmd_type, buffer, &buffer_size);
            break;
        default:
            fido_log_error("Unknown preference parameter maintenance command type");
            break;
    }
    if (ret == false) {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST + 11);
        return;
    }
    //
    // レスポンスを送信
    //  データ形式
    //  0-3: CID
    //  4:   CMD（0xc4）
    //  5-6: データサイズ（CSVデータの長さ）
    //  7:   ステータスバイト（成功時は 0x00）
    //  8-n: CSVデータ
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

static void command_erase_bonding_data(void)
{
    fido_log_info("Erase bonding data start");
    //
    // nRF52840のFlash ROM上に作成された
    // 全てのペアリング情報を削除
    //
    if (ble_service_common_erase_bond_data(command_erase_bonding_data_response) == false) {
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

void fido_maintenance_command(TRANSPORT_TYPE transport_type)
{
    // トランスポート種別を保持
    m_transport_type = transport_type;

    // リクエストデータ受信後に実行すべき処理を判定
    uint8_t cmd = get_command_byte();
    switch (cmd) {
        case MNT_COMMAND_GET_FLASH_STAT:
            command_get_flash_stat();
            break;
        case MNT_COMMAND_GET_APP_VERSION:
            command_get_app_version();
            break;
        case MNT_COMMAND_PREFERENCE_PARAM:
            command_preference_parameter_maintenance();
            break;
        case MNT_COMMAND_BOOTLOADER_MODE:
            command_bootloader_mode();
            break;
        case MNT_COMMAND_ERASE_BONDING_DATA:
            command_erase_bonding_data();
            break;
        default:
            break;
    }

    // 鍵・証明書インストール関連処理を実行
    fido_maintenance_command_skey_cert();

    // LEDをビジー状態に遷移
    fido_status_indicator_busy();
}

void fido_maintenance_command_report_sent(void)
{
    // 全フレーム送信後に行われる後続処理を実行
    uint8_t cmd = get_command_byte();
    switch (cmd) {
        case MNT_COMMAND_GET_FLASH_STAT:
            fido_log_info("Get flash ROM statistics end");
            break;
        case MNT_COMMAND_GET_APP_VERSION:
            fido_log_info("Get application version info end");
            break;
        case MNT_COMMAND_PREFERENCE_PARAM:
            fido_log_info("Preference parameter maintenance end");
            break;
        case MNT_COMMAND_BOOTLOADER_MODE:
            jump_to_bootloader_mode();
            break;
        case MNT_COMMAND_ERASE_BONDING_DATA:
            fido_log_info("Erase bonding data end");
            break;
        default:
            break;
    }

    // 鍵・証明書インストール関連処理を実行
    fido_maintenance_command_skey_cert_report_sent();

    // LEDをアイドル状態に遷移
    fido_status_indicator_idle();
}
