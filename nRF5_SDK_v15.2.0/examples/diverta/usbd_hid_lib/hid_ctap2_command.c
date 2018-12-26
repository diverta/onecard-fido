/* 
 * File:   hid_ctap2_command.c
 * Author: makmorit
 *
 * Created on 2018/12/18, 13:36
 */
#include "sdk_common.h"

#include "ctap2.h"
#include "ctap2_cbor_authgetinfo.h"
#include "ctap2_make_credential.h"
#include "fido_common.h"
#include "fido_idling_led.h"
#include "hid_fido_command.h"
#include "hid_fido_send.h"
#include "hid_fido_receive.h"
#include "usbd_hid_common.h"

// for logging informations
#define NRF_LOG_MODULE_NAME hid_ctap2_command
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

//
// CTAP2レスポンスデータ格納領域
// （コマンド共通）
//
static HID_INIT_RES_T init_res;
static uint8_t response_buffer[CTAP2_MAX_MESSAGE_SIZE];
static size_t  response_length;

void hid_ctap2_command_init(void)
{
    // 編集領域を初期化
    memset(&init_res, 0x00, sizeof(init_res));

    // nonce を取得
    uint8_t *nonce = hid_fido_receive_apdu()->data;

    // レスポンスデータを編集 (17 bytes)
    //   CIDはインクリメントされたものを設定
    memcpy(init_res.nonce, nonce, 8);
    set_CID(init_res.cid, get_incremented_CID());
    init_res.version_id    = 2;
    init_res.version_major = 5;
    init_res.version_minor = 0;
    init_res.version_build = 2;
    init_res.cflags        = CTAP2_CAPABILITY_CBOR;

    // レスポンスデータを転送
    uint32_t cid = hid_fido_receive_hid_header()->CID;
    uint8_t cmd = hid_fido_receive_hid_header()->CMD;
    hid_fido_send_command_response(cid, cmd, (uint8_t *)&init_res, sizeof(init_res));
}

static void send_ctap2_command_response(uint8_t ctap2_status, uint8_t length)
{
    // CTAP2 CBORコマンドに対応する
    // レスポンスデータを送信パケットに設定し送信
    uint32_t cid = hid_fido_receive_hid_header()->CID;
    uint32_t cmd = hid_fido_receive_hid_header()->CMD;
    // １バイトめにステータスコードをセット
    response_buffer[0] = ctap2_status;
    response_length = length;
    hid_fido_send_command_response(cid, cmd, response_buffer, response_length);

    // アイドル時点滅処理を開始
    fido_idling_led_on(LED_FOR_PROCESSING);
}

static void send_ctap2_command_error_response(uint8_t ctap2_status) 
{
    // CTAP2 CBORコマンドに対応する
    // レスポンスデータを送信パケットに設定し送信
    //   エラーなので送信バイト数＝１
    send_ctap2_command_response(ctap2_status, 1);
}

static void command_authenticator_make_credential(void)
{
    uint8_t  ctap2_status;
    uint8_t *cbor_data_buffer = hid_fido_receive_apdu()->data + 1;
    size_t   cbor_data_length = hid_fido_receive_apdu()->Lc - 1;

    // CBORエンコードされたリクエストメッセージをデコード
    ctap2_status = ctap2_make_credential_decode_request(cbor_data_buffer, cbor_data_length);
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        send_ctap2_command_error_response(ctap2_status);
    }

    // authenticatorMakeCredentialレスポンスに必要な項目を生成
    ctap2_status = ctap2_make_credential_generate_response_items();
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        send_ctap2_command_error_response(ctap2_status);
    }

    // authenticatorMakeCredentialレスポンスをエンコード
    // TODO:

    // レスポンスデータを転送
    // TODO: これは仮実装です。
    send_ctap2_command_response(CTAP1_ERR_INVALID_COMMAND, 1);
}

static void command_authenticator_get_info(void)
{
    // レスポンスの先頭１バイトはステータスコードであるため、
    // ２バイトめからCBORレスポンスをセットさせるようにする
    uint8_t  ctap2_status;
    uint8_t *cbor_data_buffer = response_buffer + 1;
    size_t   cbor_data_length = sizeof(response_buffer) - 1;
    
    // authenticatorGetInfoレスポンスをエンコード
    ctap2_status = ctap2_cbor_authgetinfo_encode_request(cbor_data_buffer, &cbor_data_length);
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        send_ctap2_command_error_response(ctap2_status);
        return;
    }

    // レスポンスデータを転送
    send_ctap2_command_response(ctap2_status, cbor_data_length + 1);
}

void hid_ctap2_command_cbor(void)
{
    // CTAP2 CBORコマンドを取得し、行うべき処理を判定
    //   最初の１バイト目がCTAP2コマンドバイトで、
    //   残りは全てCBORデータバイトとなっている
    uint8_t *ctap2_cbor_buffer = hid_fido_receive_apdu()->data;
    uint8_t  ctap2_command_byte = ctap2_cbor_buffer[0];
    switch (ctap2_command_byte) {
        case CTAP2_CMD_GETINFO:
            command_authenticator_get_info();
            break;
        case CTAP2_CMD_MAKE_CREDENTIAL:
            command_authenticator_make_credential();
            break;
        default:
            break;
    }
}
