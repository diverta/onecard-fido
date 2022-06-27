/* 
 * File:   fido_development.c
 * Author: makmorit
 *
 * Created on 2022/06/27, 10:33
 */
//
// プラットフォーム非依存コード
//
#include "fido_hid_receive.h"
#include "fido_hid_send.h"
#include "fido_development.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(fido_development);
#endif

// 関数群
static void command_install_attestation(void);
static void command_reset_attestation(void);

// トランスポート種別を保持
static TRANSPORT_TYPE m_transport_type;

static uint8_t get_command_byte(void)
{
    uint8_t cmd;
    switch (m_transport_type) {
        case TRANSPORT_HID:
            cmd = fido_hid_receive_header()->CMD;
            break;
        default:
            cmd = 0x00;
            break;
    }
    return cmd;
}

void fido_development_command(TRANSPORT_TYPE transport_type)
{
    // トランスポート種別を保持
    m_transport_type = transport_type;

    // リクエストデータ受信後に実行すべき処理を判定
    uint8_t cmd = get_command_byte();
    switch (cmd) {
        case MNT_COMMAND_INSTALL_ATTESTATION:
            command_install_attestation();
            break;
        case MNT_COMMAND_RESET_ATTESTATION:
            command_reset_attestation();
            break;
        default:
            break;
    }

    // LEDをビジー状態に遷移
    fido_status_indicator_busy();
}

void fido_development_command_report_sent(void)
{
    // 全フレーム送信後に行われる後続処理を実行
    uint8_t cmd = get_command_byte();
    switch (cmd) {
        case MNT_COMMAND_INSTALL_ATTESTATION:
            fido_log_info("Install FIDO attestation end");
            break;
        case MNT_COMMAND_RESET_ATTESTATION:
            fido_log_info("Reset FIDO attestation end");
            break;
        default:
            break;
    }

    // LEDをアイドル状態に遷移
    fido_status_indicator_idle();
}

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
    } 
}

static void send_command_error_response(uint8_t ctap2_status) 
{
    // レスポンスデータを送信パケットに設定し送信
    //   エラーなので送信バイト数＝１
    send_command_response(ctap2_status, 1);
}

//
// Install FIDO attestation
//
static void command_install_attestation(void)
{
    uint8_t *data = fido_hid_receive_apdu()->data;
    uint16_t length = fido_hid_receive_apdu()->Lc;

    // 元データチェック
    if (data == NULL || length == 0) {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST);
        return;
    }
    fido_log_info("Install FIDO attestation start");

    // Flash ROMに登録する鍵・証明書データを準備
    if (fido_flash_skey_cert_data_prepare(data, length) == false) {
        send_command_error_response(CTAP2_ERR_VENDOR_FIRST);
        return;
    }

    // TODO: 仮の実装です
    send_command_error_response(CTAP1_ERR_SUCCESS);
}

//
// Erase FIDO attestation
//
static void command_reset_attestation(void)
{
    fido_log_info("Reset FIDO attestation start");

    // TODO: 仮の実装です
    send_command_error_response(CTAP1_ERR_SUCCESS);
}
