/* 
 * File:   fido_ctap2_command.c
 * Author: makmorit
 *
 * Created on 2018/12/18, 13:36
 */
#include "sdk_common.h"

#include "ctap2_common.h"
#include "ctap2_cbor_authgetinfo.h"
#include "ctap2_make_credential.h"
#include "ctap2_get_assertion.h"
#include "ctap2_client_pin.h"
#include "ctap2_client_pin_token.h"
#include "fido_crypto_sskey.h"
#include "fido_common.h"
#include "fido_flash_password.h"
#include "hid_fido_command.h"
#include "hid_fido_send.h"
#include "hid_fido_receive.h"
#include "fido_hid_common.h"
#include "nfc_fido_receive.h"
#include "nfc_fido_send.h"

// for processing LED on/off
#include "fido_board.h"

// for ble_u2f_flash_keydata
#include "fido_flash.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_ctap2_command
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for user presence test
#include "fido_command.h"

// トランスポート種別を保持
static TRANSPORT_TYPE m_transport_type;

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
static void command_get_assertion_resume_process(void);
static void command_authenticator_reset_resume_process(void);

static uint8_t *get_cbor_data_buffer(void)
{
    uint8_t *buffer;
    switch (m_transport_type) {
        case TRANSPORT_HID:
            buffer = hid_fido_receive_apdu()->data + 1;
            break;
        case TRANSPORT_NFC:
            buffer = nfc_fido_receive_apdu()->data + 1;
            break;
        default:
            buffer = NULL;
            break;
    }
    return buffer;
}

static size_t get_cbor_data_buffer_size(void)
{
    size_t size;
    switch (m_transport_type) {
        case TRANSPORT_HID:
            size = hid_fido_receive_apdu()->Lc - 1;
            break;
        case TRANSPORT_NFC:
            size = nfc_fido_receive_apdu()->Lc + 1;
            break;
        default:
            size = 0;
            break;
    }
    return size;
}
static uint8_t get_command_byte(void)
{
    uint8_t *ctap2_cbor_buffer;
    uint8_t  ctap2_command_byte;

    // CTAP2 CBORコマンドを取得
    //   最初の１バイト目がCTAP2コマンドバイトで、
    //   残りは全てCBORデータバイトとなっている
    switch (m_transport_type) {
        case TRANSPORT_HID:
            ctap2_cbor_buffer = hid_fido_receive_apdu()->data;
            ctap2_command_byte = ctap2_cbor_buffer[0];
            break;
        case TRANSPORT_NFC:
            ctap2_cbor_buffer = nfc_fido_receive_apdu()->data;
            ctap2_command_byte = ctap2_cbor_buffer[0];
            break;
        default:
            ctap2_command_byte = 0x00;
            break;
    }

    return ctap2_command_byte;
}

static void resume_response_process(void)
{
    switch (get_command_byte()) {
        case CTAP2_CMD_MAKE_CREDENTIAL:
            NRF_LOG_INFO("authenticatorMakeCredential: completed the test of user presence");
            command_make_credential_resume_process();
            break;
        case CTAP2_CMD_GET_ASSERTION:
            NRF_LOG_INFO("authenticatorGetAssertion: completed the test of user presence");
            command_get_assertion_resume_process();
            break;
        case CTAP2_CMD_RESET:
            NRF_LOG_INFO("authenticatorReset: completed the test of user presence");
            command_authenticator_reset_resume_process();
            break;
        default:
            break;
    }
}

static void end_verify_tup(void)
{
    // ユーザー所在確認フラグをクリア
    is_tup_needed = false;
    // キープアライブを停止
    fido_user_presence_verify_end();
    // 後続のレスポンス送信処理を実行
    resume_response_process();
}

bool fido_ctap2_command_on_mainsw_event(void)
{
    if (is_tup_needed) {
        // ユーザー所在確認が必要な場合
        // (＝ユーザーによるボタン押下が行われた場合)
        // 認証処理を続行させる
        end_verify_tup();
        return true;
    }

    return false;
}

bool fido_ctap2_command_on_mainsw_long_push_event(void)
{
    // NOP
    return true;
}

void fido_ctap2_command_on_ble_nus_connected(void)
{
    if (is_tup_needed) {
        // ユーザー所在確認が必要な場合、かつ
        // One Cardのディスカバリーが成功した場合、
        // 認証処理を続行させる
        end_verify_tup();
    }
}

void fido_ctap2_command_hid_init(void)
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
    init_res.cflags        = CTAP2_CAPABILITY_WINK | CTAP2_CAPABILITY_LOCK | CTAP2_CAPABILITY_CBOR;

    // レスポンスデータを転送
    uint32_t cid = hid_fido_receive_hid_header()->CID;
    uint8_t cmd = hid_fido_receive_hid_header()->CMD;
    hid_fido_send_command_response(cid, cmd, (uint8_t *)&init_res, sizeof(init_res));
}

void fido_ctap2_command_send_response(uint8_t ctap2_status, size_t length)
{
    // CTAP2 CBORコマンドに対応する
    // レスポンスデータを送信パケットに設定し送信
    //   １バイトめにステータスコードをセット
    response_buffer[0] = ctap2_status;
    if (m_transport_type == TRANSPORT_HID) {
        uint32_t cid = hid_fido_receive_hid_header()->CID;
        uint32_t cmd = hid_fido_receive_hid_header()->CMD;
        hid_fido_send_command_response(cid, cmd, response_buffer, length);

    } else if (m_transport_type == TRANSPORT_NFC) {
        nfc_fido_send_command_response(response_buffer, length);
    } 
}

static void send_ctap2_command_error_response(uint8_t ctap2_status) 
{
    // CTAP2 CBORコマンドに対応する
    // レスポンスデータを送信パケットに設定し送信
    //   エラーなので送信バイト数＝１
    fido_ctap2_command_send_response(ctap2_status, 1);
}

static void send_ctap2_status_response(uint8_t cmd, uint8_t status_code) 
{
    // CTAP2ステータスコード（１バイト）をレスポンス
    if (m_transport_type == TRANSPORT_HID) {
        hid_fido_command_send_status_response(cmd, status_code);
    }
}

void fido_ctap2_command_keepalive_timer_handler(void)
{
    if (is_tup_needed) {
        // キープアライブ・コマンドを実行する
        if (m_transport_type == TRANSPORT_HID) {
            uint32_t cid = hid_fido_receive_hid_header()->CID;
            hid_fido_send_command_response_no_callback(cid, CTAP2_COMMAND_KEEPALIVE, CTAP2_STATUS_UPNEEDED);
        }
    }
}

static void command_authenticator_make_credential(void)
{
    // ユーザー所在確認フラグをクリア
    is_tup_needed = false;

    // CBORエンコードされたリクエストメッセージをデコード
    uint8_t *cbor_data_buffer = get_cbor_data_buffer();
    size_t   cbor_data_length = get_cbor_data_buffer_size();
    uint8_t  ctap2_status = ctap2_make_credential_decode_request(cbor_data_buffer, cbor_data_length);
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        NRF_LOG_ERROR("authenticatorMakeCredential: failed to decode CBOR request");
        send_ctap2_command_error_response(ctap2_status);
        return;
    }

    // flagsをゼロクリア
    ctap2_flags_init(0x00);

    // PINの妥当性チェック
    ctap2_status = ctap2_make_credential_verify_pin_auth();
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        send_ctap2_command_error_response(ctap2_status);
        return;
    }

    if (ctap2_make_credential_is_tup_needed()) {
        // ユーザー所在確認が必要な場合は、ここで終了し
        // その旨のフラグを設定
        is_tup_needed = true;
        // キープアライブ送信を開始
        NRF_LOG_INFO("authenticatorMakeCredential: waiting to complete the test of user presence");
        fido_user_presence_verify_start(CTAP2_KEEPALIVE_INTERVAL_MSEC, NULL);
        return;
    }

    // ユーザー所在確認不要の場合は、後続のレスポンス送信処理を実行
    command_make_credential_resume_process();
}

static void command_make_credential_resume_process(void)
{
    // LEDを消灯させる
    fido_processing_led_off();

    // 本処理を開始
    NRF_LOG_INFO("authenticatorMakeCredential start");

    // authenticatorMakeCredentialレスポンスに必要な項目を生成
    uint8_t ctap2_status = ctap2_make_credential_generate_response_items();
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        send_ctap2_command_error_response(ctap2_status);
        return;
    }

    // レスポンスの先頭１バイトはステータスコードであるため、
    // ２バイトめからCBORレスポンスをセットさせるようにする
    uint8_t *cbor_data_buffer = response_buffer + 1;
    size_t   cbor_data_length = sizeof(response_buffer) - 1;

    // authenticatorMakeCredentialレスポンスをエンコード
    ctap2_status = ctap2_make_credential_encode_response(cbor_data_buffer, &cbor_data_length);
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        send_ctap2_command_error_response(ctap2_status);
        return;
    }
    
    // レスポンス長を設定（CBORデータ長＋１）
    response_length = cbor_data_length + 1;

    // トークンカウンターレコードを追加
    // (fds_record_update/writeまたはfds_gcが実行される)
    ctap2_status = ctap2_make_credential_add_token_counter();
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        send_ctap2_command_error_response(ctap2_status);
    }
}

static void command_make_credential_send_response(fds_evt_t const *const p_evt)
{
    if (p_evt->result != FDS_SUCCESS) {
        // FDS処理でエラーが発生時は以降の処理を行わない
        send_ctap2_command_error_response(CTAP1_ERR_OTHER);
        NRF_LOG_ERROR("authenticatorMakeCredential abend: FDS EVENT=%d ", p_evt->id);
        return;
    }

    if (p_evt->id == FDS_EVT_GC) {
        // FDSリソース不足解消のためGCが実行された場合は、
        // GC実行直前の処理を再実行
        NRF_LOG_WARNING("authenticatorMakeCredential retry: FDS GC done ");
        ctap2_make_credential_add_token_counter();

    } else if (p_evt->id == FDS_EVT_UPDATE || p_evt->id == FDS_EVT_WRITE) {
        // レスポンスを生成してWebAuthnクライアントに戻す
        fido_ctap2_command_send_response(CTAP1_ERR_SUCCESS, response_length);
    }
}

static void command_authenticator_get_assertion(void)
{
    // ユーザー所在確認フラグをクリア
    is_tup_needed = false;

    // CBORエンコードされたリクエストメッセージをデコード
    uint8_t *cbor_data_buffer = get_cbor_data_buffer();
    size_t   cbor_data_length = get_cbor_data_buffer_size();
    uint8_t  ctap2_status = ctap2_get_assertion_decode_request(cbor_data_buffer, cbor_data_length);
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        NRF_LOG_ERROR("authenticatorGetAssertion: failed to decode CBOR request");
        send_ctap2_command_error_response(ctap2_status);
        return;
    }

    // flagsをゼロクリア
    ctap2_flags_init(0x00);

    // PINの妥当性チェック
    ctap2_status = ctap2_get_assertion_verify_pin_auth();
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        send_ctap2_command_error_response(ctap2_status);
        return;
    }

    if (ctap2_get_assertion_is_tup_needed()) {
        // ユーザー所在確認が必要な場合は、ここで終了し
        // その旨のフラグを設定
        is_tup_needed = true;
        // キープアライブ送信を開始
        NRF_LOG_INFO("authenticatorGetAssertion: waiting to complete the test of user presence");
        fido_user_presence_verify_start(CTAP2_KEEPALIVE_INTERVAL_MSEC, NULL);
        return;
    }

    // ユーザー所在確認不要の場合は、後続のレスポンス送信処理を実行
    command_get_assertion_resume_process();
}

static void command_get_assertion_resume_process(void)
{
    // LEDを消灯させる
    fido_processing_led_off();

    // 本処理を開始
    NRF_LOG_INFO("authenticatorGetAssertion start");

    // authenticatorGetAssertionレスポンスに必要な項目を生成
    uint8_t ctap2_status = ctap2_get_assertion_generate_response_items();
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        send_ctap2_command_error_response(ctap2_status);
        return;
    }

    // レスポンスの先頭１バイトはステータスコードであるため、
    // ２バイトめからCBORレスポンスをセットさせるようにする
    uint8_t *cbor_data_buffer = response_buffer + 1;
    size_t   cbor_data_length = sizeof(response_buffer) - 1;

    // authenticatorGetAssertionレスポンスをエンコード
    ctap2_status = ctap2_get_assertion_encode_response(cbor_data_buffer, &cbor_data_length);
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        send_ctap2_command_error_response(ctap2_status);
        return;
    }
    
    // レスポンス長を設定（CBORデータ長＋１）
    response_length = cbor_data_length + 1;

    // トークンカウンターレコードを更新
    // (fds_record_update/writeまたはfds_gcが実行される)
    ctap2_status = ctap2_get_assertion_update_token_counter();
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        send_ctap2_command_error_response(ctap2_status);
    }
}

static void command_get_assertion_send_response(fds_evt_t const *const p_evt)
{
    if (p_evt->result != FDS_SUCCESS) {
        // FDS処理でエラーが発生時は以降の処理を行わない
        send_ctap2_command_error_response(CTAP1_ERR_OTHER);
        NRF_LOG_ERROR("authenticatorGetAssertion abend: FDS EVENT=%d ", p_evt->id);
        return;
    }

    if (p_evt->id == FDS_EVT_GC) {
        // FDSリソース不足解消のためGCが実行された場合は、
        // GC実行直前の処理を再実行
        NRF_LOG_WARNING("authenticatorGetAssertion retry: FDS GC done ");
        ctap2_get_assertion_update_token_counter();

    } else if (p_evt->id == FDS_EVT_UPDATE || p_evt->id == FDS_EVT_WRITE) {
        // レスポンスを生成してWebAuthnクライアントに戻す
        fido_ctap2_command_send_response(CTAP1_ERR_SUCCESS, response_length);
    }
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
    fido_ctap2_command_send_response(ctap2_status, cbor_data_length + 1);
}

static void command_authenticator_client_pin(void)
{
    // CBORエンコードされたリクエストメッセージをデコード
    uint8_t *cbor_data_buffer = get_cbor_data_buffer();
    size_t   cbor_data_length = get_cbor_data_buffer_size();
    uint8_t  ctap2_status = ctap2_client_pin_decode_request(cbor_data_buffer, cbor_data_length);
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        NRF_LOG_ERROR("authenticatorClientPIN: failed to decode CBOR request");
        send_ctap2_command_error_response(ctap2_status);
        return;
    }

    // サブコマンドに応じた処理を実行し、
    // 処理結果のCBORレスポンスを格納
    ctap2_client_pin_perform_subcommand(response_buffer, sizeof(response_buffer));
}

static void command_authenticator_client_pin_send_response(fds_evt_t const *const p_evt)
{
    if (p_evt->result != FDS_SUCCESS) {
        // FDS処理でエラーが発生時は以降の処理を行わない
        send_ctap2_command_error_response(CTAP1_ERR_OTHER);
        NRF_LOG_ERROR("authenticatorClientPIN abend: FDS EVENT=%d ", p_evt->id);
        return;
    }

    if (p_evt->id == FDS_EVT_GC) {
        // FDSリソース不足解消のためGCが実行された場合は、
        // GC実行直前の処理を再実行
        NRF_LOG_WARNING("authenticatorClientPIN retry: FDS GC done ");
        ctap2_client_pin_perform_subcommand(response_buffer, sizeof(response_buffer));

    } else if (p_evt->id == FDS_EVT_UPDATE || p_evt->id == FDS_EVT_WRITE) {
        // リトライカウンターのFlash ROM書込処理が完了したら、
        // レスポンスを生成してCTAP2クライアントに戻す
        if (p_evt->write.record_key == FIDO_PIN_RETRY_COUNTER_RECORD_KEY) {
            ctap2_client_pin_send_response();
        }
    }
}

static void command_authenticator_reset(void)
{
    // ユーザー所在確認が必要な旨のフラグを設定
    is_tup_needed = true;
    NRF_LOG_INFO("authenticatorReset: waiting to complete the test of user presence");

    // 赤色LED高速点滅開始
    fido_processing_led_on(LED_FOR_PAIRING_MODE, LED_ON_OFF_SHORT_INTERVAL_MSEC);
}

static void command_authenticator_reset_resume_process(void)
{
    // 赤色LED高速点滅停止
    fido_processing_led_off();

    // 本処理を開始
    NRF_LOG_INFO("authenticatorReset start");

    // PINトークンとキーペアを再生成
    ctap2_client_pin_token_init(true);
    fido_crypto_sskey_init(true);

    // トークンカウンターをFlash ROM領域から削除
    // (fds_file_deleteが実行される)
    if (fido_flash_token_counter_delete() == false) {
        // NGであれば、エラーレスポンスを生成して戻す
        send_ctap2_command_error_response(CTAP1_ERR_OTHER);
        return;
    }
}

static void command_authenticator_reset_send_response(fds_evt_t const *const p_evt)
{
    if (p_evt->result != FDS_SUCCESS) {
        // FDS処理でエラーが発生時は以降の処理を行わない
        send_ctap2_command_error_response(CTAP1_ERR_OTHER);
        NRF_LOG_ERROR("authenticatorReset abend: FDS EVENT=%d ", p_evt->id);
        return;
    }

    if (p_evt->id == FDS_EVT_DEL_FILE) {
        // トークンカウンター削除完了
        NRF_LOG_DEBUG("fido_flash_token_counter_delete completed ");
        // レスポンスを生成してWebAuthnクライアントに戻す
        fido_ctap2_command_send_response(CTAP1_ERR_SUCCESS, 1);

    } else if (p_evt->id == FDS_EVT_GC) {
        // FDSリソース不足解消のためGCが実行された場合は、
        // GC実行直前の処理を再実行
        NRF_LOG_WARNING("authenticatorReset retry: FDS GC done ");
        command_authenticator_reset_resume_process();
    }
}

void fido_ctap2_command_cbor(TRANSPORT_TYPE transport_type)
{
    // トランスポート種別を保持
    m_transport_type = transport_type;
    
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
        case CTAP2_CMD_GET_ASSERTION:
            command_authenticator_get_assertion();
            break;
        case CTAP2_CMD_CLIENT_PIN:
            command_authenticator_client_pin();
            break;
        case CTAP2_CMD_RESET:
            command_authenticator_reset();
            break;
        default:
            break;
    }
}

void fido_ctap2_command_cbor_send_response(fds_evt_t const *const p_evt)
{
    // CTAP2 CBORコマンドを取得し、行うべき処理を判定
    switch (get_command_byte()) {
        case CTAP2_CMD_MAKE_CREDENTIAL:
            command_make_credential_send_response(p_evt);
            break;
        case CTAP2_CMD_GET_ASSERTION:
            command_get_assertion_send_response(p_evt);
            break;
        case CTAP2_CMD_RESET:
            command_authenticator_reset_send_response(p_evt);
            break;
        case CTAP2_CMD_CLIENT_PIN:
            command_authenticator_client_pin_send_response(p_evt);
            break;
        default:
            break;
    }
}

void fido_ctap2_command_cbor_response_completed(void)
{
    // CTAP2 CBORコマンドを取得し、行うべき処理を判定
    switch (get_command_byte()) {
        case CTAP2_CMD_MAKE_CREDENTIAL:
            NRF_LOG_INFO("authenticatorMakeCredential end");
            break;
        case CTAP2_CMD_GET_ASSERTION:
            NRF_LOG_INFO("authenticatorGetAssertion end");
            break;
        case CTAP2_CMD_RESET:
            NRF_LOG_INFO("authenticatorReset end");
            break;
        case CTAP2_CMD_CLIENT_PIN:
            NRF_LOG_INFO("authenticatorClientPIN end");
            break;
        default:
            break;
    }
}

void fido_ctap2_command_tup_cancel(void)
{
    if (is_tup_needed) {
        // ユーザー所在確認待ちの場合はキャンセル
        is_tup_needed = false;
        fido_user_presence_verify_end();
        NRF_LOG_INFO("canceled the test of user presence");
    }
}

void fido_ctap2_command_cancel(void)
{
    if (is_tup_needed) {
        // ユーザー所在確認待ちの場合はキャンセル
        is_tup_needed = false;
        fido_user_presence_verify_end();

        // キャンセルレスポンスを戻す
        //   CMD:    CTAPHID_CBOR
        //   status: CTAP2_ERR_KEEPALIVE_CANCEL
        send_ctap2_status_response(CTAP2_COMMAND_CBOR, CTAP2_ERR_KEEPALIVE_CANCEL);
        NRF_LOG_INFO("CTAPHID_CANCEL done with CBOR command");
    }
}
