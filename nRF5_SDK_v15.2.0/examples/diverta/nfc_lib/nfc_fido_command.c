/* 
 * File:   nfc_fido_command.c
 * Author: makmorit
 *
 * Created on 2019/06/03, 15:20
 */
#include "sdk_common.h"

#include "fido_common.h"
#include "ctap2_common.h"
#include "hid_ctap2_command.h"

#include "nfc_fido_receive.h"
#include "nfc_fido_send.h"

// for logging informations
#define NRF_LOG_MODULE_NAME nfc_fido_command
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

static uint8_t get_command_byte(void)
{
    // CTAP2 CBORコマンドを取得
    //   最初の１バイト目がCTAP2コマンドバイトで、
    //   残りは全てCBORデータバイトとなっている
    uint8_t *ctap2_cbor_buffer = nfc_fido_receive_apdu()->data;
    return ctap2_cbor_buffer[0];
}

void nfc_fido_command_on_request_received(void)
{
    // CTAP2 CBORコマンドを取得し、行うべき処理を判定
    //   最初の１バイト目がCTAP2コマンドバイトで、
    //   残りは全てCBORデータバイトとなっている
    switch (get_command_byte()) {
        case CTAP2_COMMAND_CBOR:
            fido_ctap2_command_cbor(TRANSPORT_NFC);
            break;
        default:
            // 不正なコマンドであるため
            // エラーレスポンスを送信
            NRF_LOG_ERROR("Invalid command (0x%02x) ", get_command_byte());
            nfc_fido_send_response(SW_INS_INVALID);
            break;
    }
}

void nfc_fido_command_on_fs_evt(fds_evt_t const *const p_evt)
{
    // Flash ROM更新完了時の処理を実行
    uint8_t cmd = nfc_fido_receive_apdu()->INS;
    switch (cmd) {
        case CTAP2_COMMAND_CBOR:
            fido_ctap2_command_cbor_send_response(p_evt);
            break;
        default:
            break;
    }
}
