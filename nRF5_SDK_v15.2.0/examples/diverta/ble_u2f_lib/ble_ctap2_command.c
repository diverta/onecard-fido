/* 
 * File:   ble_ctap2_command.c
 * Author: makmorit
 *
 * Created on 2019/04/30, 13:48
 */
#include "sdk_common.h"

#include "ctap2_common.h"
#include "ctap2_cbor_authgetinfo.h"
#include "ctap2_client_pin_token.h"
#include "ctap2_client_pin_sskey.h"
#include "fido_common.h"

// for BLE transport
#include "ble_u2f_command.h"
#include "ble_u2f_status.h"

// for processing LED on/off
#include "fido_processing_led.h"

// for ble_u2f_flash_keydata
#include "fido_flash.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ble_ctap2_command
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for user presence test
#include "fido_user_presence.h"

// ユーザー所在確認が必要かどうかを保持
static bool is_tup_needed = false;
//
// CTAP2レスポンスデータ格納領域
// （コマンド共通）
//
static uint8_t response_buffer[CTAP2_MAX_MESSAGE_SIZE];
static size_t  response_length;

static void ble_ctap2_command_send_response(uint8_t ctap2_status, size_t length)
{
    // コマンドを格納
    ble_u2f_context_t *p_u2f_context = get_ble_u2f_context();
    uint8_t command_for_response = p_u2f_context->p_ble_header->CMD;
    // １バイトめにステータスコードをセット
    response_buffer[0] = ctap2_status;
    response_length = length;
    
    // レスポンスを送信
    ble_u2f_status_setup(command_for_response, response_buffer, response_length);
    ble_u2f_status_response_send(p_u2f_context->p_u2f);
}

static void send_ctap2_command_error_response(uint8_t ctap2_status) 
{
    // CTAP2 CBORコマンドに対応する
    // レスポンスデータを送信パケットに設定し送信
    //   エラーなので送信バイト数＝１
    ble_ctap2_command_send_response(ctap2_status, 1);
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
    ble_ctap2_command_send_response(ctap2_status, cbor_data_length + 1);
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
    ctap2_client_pin_sskey_init(true);

    // トークンカウンターをFlash ROM領域から削除
    // (fds_file_deleteが実行される)
    if (fido_flash_token_counter_delete() == false) {
        // NGであれば、エラーレスポンスを生成して戻す
        send_ctap2_command_error_response(CTAP1_ERR_OTHER);
        return;
    }
}

static void command_authenticator_reset_send_response(fds_evt_t const *p_evt)
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
        send_ctap2_command_error_response(CTAP1_ERR_SUCCESS);

    } else if (p_evt->id == FDS_EVT_GC) {
        // FDSリソース不足解消のためGCが実行された場合は、
        // GC実行直前の処理を再実行
        NRF_LOG_WARNING("authenticatorReset retry: FDS GC done ");
        command_authenticator_reset_resume_process();
    }
}

static uint8_t get_command_byte(void)
{
    // CTAP2コマンドを取得
    //   受信データの最初の１バイト目がCTAP2コマンドバイト
    ble_u2f_context_t *p_u2f_context = get_ble_u2f_context();
    uint8_t ctap2_command_byte = p_u2f_context->p_apdu->CLA;
    return ctap2_command_byte;
}

static void resume_response_process(void)
{
    switch (get_command_byte()) {
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

bool ble_ctap2_command_on_mainsw_event(void)
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

bool is_ctap2_command_byte(uint8_t command_byte)
{
    // CTAP2のコマンドバイトかどうか判定する
    bool ret = false;
    switch (command_byte) {
        case CTAP2_CMD_GETINFO:
        case CTAP2_CMD_RESET:
            ret = true;
            break;
        default:
            break;
    }
    return ret;
}

void ble_ctap2_command_do_process(void)
{
    switch (get_command_byte()) {
        case CTAP2_CMD_GETINFO:
            command_authenticator_get_info();
            break;
        case CTAP2_CMD_RESET:
            command_authenticator_reset();
            break;
        default:
            break;
    }
}

void ble_ctap2_command_on_fs_evt(fds_evt_t const *const p_evt)
{
    // CTAP2 CBORコマンドを取得し、行うべき処理を判定
    switch (get_command_byte()) {
        case CTAP2_CMD_RESET:
            command_authenticator_reset_send_response(p_evt);
            break;
        default:
            break;
    }
}

void ble_ctap2_command_response_sent(void)
{
    // CTAP2 CBORコマンドを取得し、行うべき処理を判定
    switch (get_command_byte()) {
        case CTAP2_CMD_GETINFO:
            NRF_LOG_INFO("authenticatorGetInfo end");
            break;
        case CTAP2_CMD_RESET:
            NRF_LOG_INFO("authenticatorReset end");
            break;
        default:
            break;
    }
}
