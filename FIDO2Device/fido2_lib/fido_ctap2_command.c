/* 
 * File:   fido_ctap2_command.c
 * Author: makmorit
 *
 * Created on 2018/12/18, 13:36
 */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

//
// プラットフォーム非依存コード
//
#include "ctap2_cbor_authgetinfo.h"
#include "ctap2_client_pin.h"
#include "ctap2_client_pin_token.h"
#include "ctap2_common.h"
#include "ctap2_define.h"
#include "ctap2_get_assertion.h"
#include "ctap2_make_credential.h"
#include "fido_command.h"
#include "fido_command_common.h"
#include "fido_common.h"
#include "fido_define.h"
#include "fido_ble_receive.h"
#include "fido_ble_send.h"
#include "fido_hid_channel.h"
#include "fido_hid_send.h"
#include "fido_hid_receive.h"
#include "fido_transport_define.h"
#include "u2f_define.h"
#include "ctap2_pubkey_credential.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(fido_ctap2_command);
#endif

// トランスポート種別を保持
static TRANSPORT_TYPE m_transport_type;

// ユーザー所在確認が必要かどうかを保持
static bool is_tup_needed = false;

//
// CTAP2レスポンスデータ格納領域
// （コマンド共通）
//
static HID_INIT_RES_T init_res;

// 関数プロトタイプ
static void command_make_credential_resume_process(void);
static void command_get_assertion_resume_process(void);
static void command_authenticator_reset_resume_process(void);

static uint8_t *get_cbor_data_buffer(void)
{
    uint8_t *buffer;
    switch (m_transport_type) {
        case TRANSPORT_HID:
            buffer = fido_hid_receive_apdu_data() + 1;
            break;
        case TRANSPORT_BLE:
            buffer = fido_ble_receive_apdu_data() + 1;
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
            size = fido_hid_receive_apdu_Lc() - 1;
            break;
        case TRANSPORT_BLE:
            size = fido_ble_receive_apdu_Lc() - 1;
            break;
        default:
            size = 0;
            break;
    }
    return size;
}
static uint8_t get_ctap2_command_byte(void)
{
    uint8_t *ctap2_cbor_buffer;
    uint8_t  ctap2_command_byte;

    // CTAP2 CBORコマンドを取得
    //   最初の１バイト目がCTAP2コマンドバイトで、
    //   残りは全てCBORデータバイトとなっている
    switch (m_transport_type) {
        case TRANSPORT_HID:
            ctap2_cbor_buffer = fido_hid_receive_apdu_data();
            ctap2_command_byte = ctap2_cbor_buffer[0];
            break;
        case TRANSPORT_BLE:
            ctap2_cbor_buffer = fido_ble_receive_apdu_data();
            ctap2_command_byte = ctap2_cbor_buffer[0];
            break;
        default:
            ctap2_command_byte = 0x00;
            break;
    }

    return ctap2_command_byte;
}

static void resume_response_process(bool tup_done)
{
    // LEDをビジー状態に遷移
    fido_status_indicator_busy();

    switch (get_ctap2_command_byte()) {
        case CTAP2_CMD_MAKE_CREDENTIAL:
            // `ctap2_make_credential_generate_response_items`内で実行される
            // `fido_command_generate_random_vector`の実行事前に、
            // ランダムベクターの生成を指示
            fido_user_presence_verify_end_message("authenticatorMakeCredential", tup_done);
            fido_crypto_random_pre_generate(command_make_credential_resume_process);
            break;
        case CTAP2_CMD_GET_ASSERTION:
            fido_user_presence_verify_end_message("authenticatorGetAssertion", tup_done);
            command_get_assertion_resume_process();
            break;
        case CTAP2_CMD_RESET:
            // `ctap2_client_pin_init`内で実行される
            // `fido_command_generate_random_vector`の実行事前に、
            // ランダムベクターの生成を指示
            fido_user_presence_verify_end_message("authenticatorReset", tup_done);
            fido_crypto_random_pre_generate(command_authenticator_reset_resume_process);
            break;
        default:
            break;
    }
}

bool fido_ctap2_command_on_mainsw_event(void)
{
    if (is_tup_needed) {
        // ユーザー所在確認が必要な場合
        // (＝ユーザーによるボタン押下が行われた場合)
        // ユーザー所在確認フラグをクリア
        is_tup_needed = false;
        // キープアライブを停止
        fido_user_presence_verify_end();
        // 後続のレスポンス送信処理を実行
        resume_response_process(true);
        return true;
    }

    return false;
}

bool fido_ctap2_command_on_mainsw_long_push_event(void)
{
    // NOP
    return true;
}

//
// USB HID専用コマンド群
//
void fido_ctap2_command_hid_init(void)
{
    // 編集領域を初期化
    memset(&init_res, 0x00, sizeof(init_res));

    // nonce を取得
    uint8_t *nonce = fido_hid_receive_apdu_data();

    // レスポンスデータを編集 (17 bytes)
    //   CIDはインクリメントされたものを設定
    memcpy(init_res.nonce, nonce, 8);
    fido_hid_channel_set_cid_bytes(init_res.cid, fido_hid_channel_new_cid());
    init_res.version_id    = 2;
    init_res.version_major = 5;
    init_res.version_minor = 0;
    init_res.version_build = 2;
    init_res.cflags        = CTAP2_CAPABILITY_WINK | CTAP2_CAPABILITY_LOCK | CTAP2_CAPABILITY_CBOR;

    // レスポンスデータを転送
    uint32_t cid = fido_hid_receive_header_CID();
    uint8_t  cmd = fido_hid_receive_header_CMD();
    fido_hid_send_command_response(cid, cmd, (uint8_t *)&init_res, sizeof(init_res));
}

void fido_ctap2_command_wink(void)
{
    // ステータスなしでレスポンスする
    uint32_t cid = fido_hid_receive_header_CID();
    uint8_t  cmd = fido_hid_receive_header_CMD();
    fido_hid_send_command_response_no_payload(cid, cmd);
}

void fido_ctap2_command_lock(void)
{
    // ロックコマンドのパラメーターを取得する
    uint32_t cid = fido_hid_receive_header_CID();
    uint8_t  cmd = fido_hid_receive_header_CMD();
    uint8_t  lock_param = fido_hid_receive_apdu_data()[0];

    if (lock_param > 0) {
        // パラメーターが指定されていた場合
        // ロック対象CIDを設定
        fido_hid_channel_lock_start(cid, lock_param);

    } else {
        // CIDのロックを解除
        fido_hid_channel_lock_cancel();
    }

    // ステータスなしでレスポンスする
    fido_hid_send_command_response_no_payload(cid, cmd);
}

void fido_ctap2_command_send_response(uint8_t ctap2_status, size_t length)
{
    // CTAP2 CBORコマンドに対応する
    // レスポンスデータを送信パケットに設定し送信
    //   １バイトめにステータスコードをセット
    uint8_t *response_buffer = fido_command_response_data();
    response_buffer[0] = ctap2_status;
    if (m_transport_type == TRANSPORT_HID) {
        uint32_t cid = fido_hid_receive_header_CID();
        uint32_t cmd = fido_hid_receive_header_CMD();
        fido_hid_send_command_response(cid, cmd, response_buffer, length);

    } else if (m_transport_type == TRANSPORT_BLE) {
        uint8_t cmd = fido_ble_receive_header_CMD();
        fido_ble_send_command_response(cmd, response_buffer, length);
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
        fido_hid_send_status_response(cmd, status_code);

    } else if (m_transport_type == TRANSPORT_BLE) {
        fido_ble_send_status_response(cmd, status_code);
    }
}

void fido_ctap2_command_keepalive_timer_handler(void)
{
    if (is_tup_needed) {
        // キープアライブ・コマンドを実行する
        if (m_transport_type == TRANSPORT_HID) {
            fido_hid_send_status_response(CTAP2_COMMAND_KEEPALIVE, CTAP2_STATUS_UPNEEDED);

        } else if (m_transport_type == TRANSPORT_BLE) {
            fido_ble_send_status_response(U2F_COMMAND_KEEPALIVE, CTAP2_STATUS_UPNEEDED);
        }
    }
}

static void command_authenticator_make_credential(void)
{
    // ユーザー所在確認フラグをクリア
    is_tup_needed = false;

    if (fido_command_check_skey_cert_exist() == false) {
        // 秘密鍵と証明書がFlash ROMに登録されていない場合
        // エラーレスポンスを生成して戻す
        fido_log_error("authenticatorMakeCredential: private key and certification not available");
        send_ctap2_command_error_response(CTAP2_ERR_VENDOR_KEY_CRT_NOT_EXIST);
        return;
    }

    if (fido_command_check_aes_password_exist() == false) {
        // キーハンドルを暗号化するために必要な
        // AESパスワードが生成されていない場合
        // エラーレスポンスを生成して戻す
        fido_log_error("authenticatorMakeCredential: AES password is not exist");
        send_ctap2_command_error_response(CTAP2_ERR_VENDOR_KEY_CRT_NOT_EXIST);
        return;
    }

    // CBORエンコードされたリクエストメッセージをデコード
    uint8_t *cbor_data_buffer = get_cbor_data_buffer();
    size_t   cbor_data_length = get_cbor_data_buffer_size();
    uint8_t  ctap2_status = ctap2_make_credential_decode_request(cbor_data_buffer, cbor_data_length);
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        fido_log_error("authenticatorMakeCredential: failed to decode CBOR request");
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
        fido_log_info("authenticatorMakeCredential: waiting to complete the test of user presence");
        fido_user_presence_verify_start(CTAP2_KEEPALIVE_INTERVAL_MSEC, NULL);
        return;
    }

    // ユーザー所在確認不要の場合は、後続のレスポンス送信処理を実行
    resume_response_process(false);
}

static void command_make_credential_resume_process(void)
{
    // 本処理を開始
    fido_log_info("authenticatorMakeCredential start");

    // authenticatorMakeCredentialレスポンスに必要な項目を生成
    uint8_t ctap2_status = ctap2_make_credential_generate_response_items();
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        send_ctap2_command_error_response(ctap2_status);
        return;
    }

    // レスポンスの先頭１バイトはステータスコードであるため、
    // ２バイトめからCBORレスポンスをセットさせるようにする
    uint8_t *response_buffer = fido_command_response_data();
    uint8_t *cbor_data_buffer = response_buffer + 1;
    size_t   cbor_data_length = fido_command_response_data_size_max() - 1;

    // authenticatorMakeCredentialレスポンスをエンコード
    ctap2_status = ctap2_make_credential_encode_response(cbor_data_buffer, &cbor_data_length);
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        send_ctap2_command_error_response(ctap2_status);
        return;
    }
    
    // レスポンス長を設定（CBORデータ長＋１）
    fido_command_response_data_size_set(cbor_data_length + 1);

    // トークンカウンターレコードを追加
    // (fds_record_update/writeまたはfds_gcが実行される)
    ctap2_status = ctap2_make_credential_add_token_counter();
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        send_ctap2_command_error_response(ctap2_status);
    }
}

static void command_authenticator_get_assertion(void)
{
    // ユーザー所在確認フラグをクリア
    is_tup_needed = false;

    if (fido_command_check_aes_password_exist() == false) {
        // キーハンドルを復号化するために必要な
        // AESパスワードが生成されていない場合
        // エラーレスポンスを生成して戻す
        fido_log_error("authenticatorGetAssertion: AES password is not exist");
        send_ctap2_command_error_response(CTAP2_ERR_VENDOR_KEY_CRT_NOT_EXIST);
        return;
    }

    // CBORエンコードされたリクエストメッセージをデコード
    uint8_t *cbor_data_buffer = get_cbor_data_buffer();
    size_t   cbor_data_length = get_cbor_data_buffer_size();
    uint8_t  ctap2_status = ctap2_get_assertion_decode_request(cbor_data_buffer, cbor_data_length);
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        fido_log_error("authenticatorGetAssertion: failed to decode CBOR request");
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
        fido_log_info("authenticatorGetAssertion: waiting to complete the test of user presence");
        fido_user_presence_verify_start(CTAP2_KEEPALIVE_INTERVAL_MSEC,
            ctap2_pubkey_credential_ble_auth_scan_param());
        return;
    }

    // ユーザー所在確認不要の場合は、後続のレスポンス送信処理を実行
    resume_response_process(false);
}

static void command_get_assertion_resume_process(void)
{
    // 本処理を開始
    fido_log_info("authenticatorGetAssertion start");

    // authenticatorGetAssertionレスポンスに必要な項目を生成
    uint8_t ctap2_status = ctap2_get_assertion_generate_response_items();
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        send_ctap2_command_error_response(ctap2_status);
        return;
    }

    // レスポンスの先頭１バイトはステータスコードであるため、
    // ２バイトめからCBORレスポンスをセットさせるようにする
    uint8_t *response_buffer = fido_command_response_data();
    uint8_t *cbor_data_buffer = response_buffer + 1;
    size_t   cbor_data_length = fido_command_response_data_size_max() - 1;

    // authenticatorGetAssertionレスポンスをエンコード
    ctap2_status = ctap2_get_assertion_encode_response(cbor_data_buffer, &cbor_data_length);
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        send_ctap2_command_error_response(ctap2_status);
        return;
    }
    
    // レスポンス長を設定（CBORデータ長＋１）
    fido_command_response_data_size_set(cbor_data_length + 1);

    // トークンカウンターレコードを更新
    // (fds_record_update/writeまたはfds_gcが実行される)
    ctap2_status = ctap2_get_assertion_update_token_counter();
    if (ctap2_status != CTAP1_ERR_SUCCESS) {
        // NGであれば、エラーレスポンスを生成して戻す
        send_ctap2_command_error_response(ctap2_status);
    }
}

static void command_authenticator_get_info(void)
{
    // レスポンスの先頭１バイトはステータスコードであるため、
    // ２バイトめからCBORレスポンスをセットさせるようにする
    uint8_t  ctap2_status;
    uint8_t *response_buffer = fido_command_response_data();
    uint8_t *cbor_data_buffer = response_buffer + 1;
    size_t   cbor_data_length = fido_command_response_data_size_max() - 1;
    
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
        fido_log_error("authenticatorClientPIN: failed to decode CBOR request");
        send_ctap2_command_error_response(ctap2_status);
        return;
    }

    // サブコマンドに応じた処理を実行し、
    // 処理結果のCBORレスポンスを格納
    uint8_t *response_buffer = fido_command_response_data();
    ctap2_client_pin_perform_subcommand(response_buffer, fido_command_response_data_size_max());
}

static void command_authenticator_reset(void)
{
    // ユーザー所在確認が必要な旨のフラグを設定
    is_tup_needed = true;
    fido_log_info("authenticatorReset: waiting to complete the test of user presence");

    // 赤色LED高速点滅開始し、ボタン押下を待つ
    fido_user_presence_verify_start_on_reset();
}

static void command_authenticator_reset_resume_process(void)
{
    // 本処理を開始
    fido_log_info("authenticatorReset start");

    // PINトークンとキーペアを再生成
    ctap2_client_pin_token_init(true);
    fido_command_sskey_init(true);

    // 署名カウンター情報をFlash ROM領域から削除
    // (fds_file_deleteが実行される)
    if (fido_command_sign_counter_delete() == false) {
        // NGであれば、エラーレスポンスを生成して戻す
        send_ctap2_command_error_response(CTAP1_ERR_OTHER);
        return;
    }
}

static void fido_ctap2_command_cbor(TRANSPORT_TYPE transport_type)
{
    // トランスポート種別を保持
    m_transport_type = transport_type;
    
    // CTAP2 CBORコマンドを取得し、行うべき処理を判定
    //   最初の１バイト目がCTAP2コマンドバイトで、
    //   残りは全てCBORデータバイトとなっている
    switch (get_ctap2_command_byte()) {
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

void fido_ctap2_command_cbor_ble()
{
    fido_ctap2_command_cbor(TRANSPORT_BLE);
}

void fido_ctap2_command_cbor_hid()
{
    fido_ctap2_command_cbor(TRANSPORT_HID);
}

static bool verify_ctap2_cbor_command(void)
{
    switch (m_transport_type) {
        case TRANSPORT_HID:
            if (fido_hid_receive_header_CMD() == CTAP2_COMMAND_CBOR) {
                return true;
            }
            break;
        case TRANSPORT_BLE:
            if (fido_ble_receive_header_CMD() == U2F_COMMAND_MSG) {
                // BLE CTAP2 command
                return true;
            }
            break;
        default:
            break;
    }
    return false;
}

void fido_ctap2_command_tup_cancel(void)
{
    if (is_tup_needed) {
        // ユーザー所在確認待ちの場合はキャンセル
        is_tup_needed = false;
        fido_log_info("Canceled the CTAP2 test of user presence");
    }
}

void fido_ctap2_command_cancel(void)
{
    if (is_tup_needed) {
        // ユーザー所在確認待ちの場合はキャンセル
        is_tup_needed = false;
        fido_user_presence_verify_cancel();

        // キャンセルレスポンスを戻す
        //   CMD:    CTAPHID_CBOR
        //   status: CTAP2_ERR_KEEPALIVE_CANCEL
        send_ctap2_status_response(CTAP2_COMMAND_CBOR, CTAP2_ERR_KEEPALIVE_CANCEL);
        fido_log_info("CTAPHID_CANCEL done with CBOR command");
    }
}

void fido_ctap2_command_flash_failed(void)
{
    if (verify_ctap2_cbor_command() == false) {
        // CTAP2 CBORコマンド以外は処理しない
        return;
    }
    
    // Flash ROM処理でエラーが発生時はエラーレスポンス送信
    switch (get_ctap2_command_byte()) {
        case CTAP2_CMD_MAKE_CREDENTIAL:
            send_ctap2_command_error_response(CTAP1_ERR_OTHER);
            fido_log_error("authenticatorMakeCredential abend");
            break;
        case CTAP2_CMD_GET_ASSERTION:
            send_ctap2_command_error_response(CTAP1_ERR_OTHER);
            fido_log_error("authenticatorGetAssertion abend");
            break;
        case CTAP2_CMD_RESET:
            send_ctap2_command_error_response(CTAP1_ERR_OTHER);
            fido_log_error("authenticatorReset abend");
            break;
        case CTAP2_CMD_CLIENT_PIN:
            send_ctap2_command_error_response(CTAP1_ERR_OTHER);
            fido_log_error("authenticatorClientPIN abend");
            break;
        default:
            break;
    }
}

void fido_ctap2_command_flash_gc_done(void)
{
    if (verify_ctap2_cbor_command() == false) {
        // CTAP2 CBORコマンド以外は処理しない
        return;
    }
    
    // for nRF52840:
    // FDSリソース不足解消のためGCが実行された場合は、
    // GC実行直前の処理を再実行
    switch (get_ctap2_command_byte()) {
        case CTAP2_CMD_MAKE_CREDENTIAL:
            fido_log_warning("authenticatorMakeCredential retry: FDS GC done ");
            ctap2_make_credential_add_token_counter();
            break;
        case CTAP2_CMD_GET_ASSERTION:
            fido_log_warning("authenticatorGetAssertion retry: FDS GC done ");
            ctap2_get_assertion_update_token_counter();
            break;
        case CTAP2_CMD_RESET:
            fido_log_warning("authenticatorReset retry: FDS GC done ");
            command_authenticator_reset_resume_process();
            break;
        case CTAP2_CMD_CLIENT_PIN:
            fido_log_warning("authenticatorClientPIN retry: FDS GC done ");
            ctap2_client_pin_perform_subcommand(fido_command_response_data(), fido_command_response_data_size_max());
            break;
        default:
            break;
    }
}

void fido_ctap2_command_token_counter_file_deleted(void)
{
    if (verify_ctap2_cbor_command()) {
        if (get_ctap2_command_byte() == CTAP2_CMD_RESET) {
            // トークンカウンター削除完了
            fido_log_debug("authenticatorReset: Erase token counter file completed");
            // レスポンスを生成してWebAuthnクライアントに戻す
            fido_ctap2_command_send_response(CTAP1_ERR_SUCCESS, 1);
        }
    }
}

void fido_ctap2_command_retry_counter_record_updated(void)
{
    if (verify_ctap2_cbor_command()) {
        if (get_ctap2_command_byte() == CTAP2_CMD_CLIENT_PIN) {
            // リトライカウンターのFlash ROM書込処理が完了したら、
            // レスポンスを生成してCTAP2クライアントに戻す
            ctap2_client_pin_send_response();
        }
    }
}

void fido_ctap2_command_token_counter_record_updated(void)
{
    if (verify_ctap2_cbor_command() == false) {
        // CTAP2 CBORコマンド以外は処理しない
        return;
    }
    
    // CTAP2 CBORコマンドを取得し、行うべき処理を判定
    switch (get_ctap2_command_byte()) {
        case CTAP2_CMD_MAKE_CREDENTIAL:
            // レスポンスを生成してWebAuthnクライアントに戻す
            fido_ctap2_command_send_response(CTAP1_ERR_SUCCESS, fido_command_response_data_size());
            break;
        case CTAP2_CMD_GET_ASSERTION:
            // レスポンスを生成してWebAuthnクライアントに戻す
            fido_ctap2_command_send_response(CTAP1_ERR_SUCCESS, fido_command_response_data_size());
            break;
        default:
            break;
    }
}

void fido_ctap2_command_init_response_sent(void)
{
    fido_log_debug("CTAPHID_INIT end");
}

void fido_ctap2_command_cbor_response_sent(void)
{
    // LEDをアイドル状態に遷移
    fido_status_indicator_idle();

    // CTAP2 CBORコマンドを取得し、行うべき処理を判定
    switch (get_ctap2_command_byte()) {
        case CTAP2_CMD_GETINFO:
            fido_log_info("authenticatorGetInfo end");
            break;
        case CTAP2_CMD_MAKE_CREDENTIAL:
            fido_log_info("authenticatorMakeCredential end");
            break;
        case CTAP2_CMD_GET_ASSERTION:
            fido_log_info("authenticatorGetAssertion end");
            break;
        case CTAP2_CMD_RESET:
            fido_log_info("authenticatorReset end");
            break;
        case CTAP2_CMD_CLIENT_PIN:
            ctap2_client_pin_response_sent();
            break;
        default:
            break;
    }
}
