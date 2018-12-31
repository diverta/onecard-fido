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

// for processing LED on/off
#include "fido_processing_led.h"

// for logging informations
#define NRF_LOG_MODULE_NAME hid_ctap2_command
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// ユーザー所在確認が必要かどうかを保持
static bool is_tup_needed = false;

//
// CTAP2レスポンスデータ格納領域
// （コマンド共通）
//
static HID_INIT_RES_T init_res;
static uint8_t response_buffer[CTAP2_MAX_MESSAGE_SIZE];
static size_t  response_length;

// 関数プロトタイプ
static void command_make_credential_resume_process(void);

static uint8_t get_command_byte(void)
{
    // CTAP2 CBORコマンドを取得
    //   最初の１バイト目がCTAP2コマンドバイトで、
    //   残りは全てCBORデータバイトとなっている
    uint8_t *ctap2_cbor_buffer = hid_fido_receive_apdu()->data;
    uint8_t  ctap2_command_byte = ctap2_cbor_buffer[0];

    return ctap2_command_byte;
}

static void resume_response_process(void)
{
    switch (get_command_byte()) {
        case CTAP2_CMD_MAKE_CREDENTIAL:
            NRF_LOG_INFO("authenticatorMakeCredential: completed the test of user presence");
            command_make_credential_resume_process();
            break;
        default:
            break;
    }
}

bool hid_ctap2_command_on_mainsw_event(void)
{
    if (is_tup_needed) {
        // ユーザー所在確認が必要な場合
        // (＝ユーザーによるボタン押下が行われた場合)
        is_tup_needed = false;
        // LEDを消灯させる
        fido_processing_led_off();
        // 後続のレスポンス送信処理を実行
        resume_response_process();
        return true;
    }

    return false;
}

bool hid_ctap2_command_on_mainsw_long_push_event(void)
{
    // NOP
    return true;
}

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
    // ユーザー所在確認フラグをクリア
    is_tup_needed = false;

    NRF_LOG_INFO("authenticatorMakeCredential start");

    // 秘密鍵と証明書をFlash ROMから読込
    // 秘密鍵と証明書がFlash ROMに登録されていない場合
    // エラーレスポンスを生成して戻す
    // TODO:

    // CBORエンコードされたリクエストメッセージをデコード
    uint8_t *cbor_data_buffer = hid_fido_receive_apdu()->data + 1;
    size_t   cbor_data_length = hid_fido_receive_apdu()->Lc - 1;
    uint8_t  ctap2_status = ctap2_make_credential_decode_request(cbor_data_buffer, cbor_data_length);
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        send_ctap2_command_error_response(ctap2_status);
    }

    if (ctap2_make_credential_is_tup_needed()) {
        // ユーザー所在確認が必要な場合は、ここで終了し
        // その旨のフラグを設定
        is_tup_needed = true;
        NRF_LOG_INFO("authenticatorMakeCredential: waiting to complete the test of user presence");
        // LED点滅を開始
        fido_processing_led_on(LED_FOR_USER_PRESENCE);
        return;
    }

    // ユーザー所在確認不要の場合は、後続のレスポンス送信処理を実行
    command_make_credential_resume_process();
}

static void command_make_credential_resume_process(void)
{
    // authenticatorMakeCredentialレスポンスに必要な項目を生成
    uint8_t ctap2_status = ctap2_make_credential_generate_response_items();
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        send_ctap2_command_error_response(ctap2_status);
    }

    // authenticatorMakeCredentialレスポンスをエンコード
    // TODO:

    // レスポンスデータを転送
    // TODO: これは仮実装です。
    send_ctap2_command_response(CTAP1_ERR_INVALID_COMMAND, 1);
    NRF_LOG_INFO("authenticatorMakeCredential end");
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
    switch (get_command_byte()) {
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
