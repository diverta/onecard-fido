#include "sdk_common.h"

#include <stdio.h>
#include <string.h>
#include "ble_u2f_securekey.h"
#include "ble_u2f_crypto.h"
#include "ble_u2f_flash.h"
#include "ble_u2f_status.h"
#include "ble_u2f_command.h"
#include "ble_u2f_user_presence.h"
#include "ble_u2f_util.h"
#include "u2f_keyhandle.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ble_u2f_authenticate
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();


static uint8_t *get_appid_from_apdu(ble_u2f_context_t *p_u2f_context)
{
    // appIdHashを参照し、APDUの33バイト目のアドレスを戻す
    FIDO_APDU_T *p_apdu = p_u2f_context->p_apdu;
    uint8_t *p_appid_hash = p_apdu->data + U2F_CHAL_SIZE;

    return p_appid_hash;
}

static bool check_request_keyhandle(ble_u2f_context_t *p_u2f_context)
{
    // リクエストデータのキーハンドルを参照
    //   APDUの65バイト目以降
    FIDO_APDU_T *p_apdu = p_u2f_context->p_apdu;
    uint32_t keyhandle_length = p_apdu->data[64];
    uint8_t *keyhandle_value = p_apdu->data + 65;

    // キーハンドルを復号化
    //   keyhandle_base_bufferに
    //   AppIDHash、秘密鍵が格納される
    u2f_keyhandle_restore(keyhandle_value, keyhandle_length);
    
    // リクエストデータからappIDHashを取得
    // キーハンドルに含まれているものと異なる場合はエラー
    uint8_t *p_appid_hash = get_appid_from_apdu(p_u2f_context);
    if (strncmp((char *)keyhandle_base_buffer, (char *)p_appid_hash, U2F_APPID_SIZE) != 0) {
        return false;
    }

    return true;
}

static void update_token_counter(ble_u2f_context_t *p_u2f_context)
{
    // 開始ログを出力
    NRF_LOG_DEBUG("update_token_counter start ");

    // appIdHash、トークンカウンターを共有情報から取得
    // （トークンカウンターは現在値＋１とする）
    uint8_t *p_appid_hash = get_appid_from_apdu(p_u2f_context);
    uint32_t token_counter = p_u2f_context->token_counter + 1;

    // appIdHashをキーとして、
    // トークンカウンターレコードを更新する
    uint32_t reserve_word = 0xffffffff;
    if (ble_u2f_flash_token_counter_write(p_u2f_context, p_appid_hash, token_counter, reserve_word) == false) {
        // NGであれば、エラーレスポンスを生成して終了
        ble_u2f_send_error_response(p_u2f_context, 0x9502);
        return;
    }

    // 後続のレスポンス生成・送信は、
    // Flash ROM書込み完了後に行われる
    NRF_LOG_DEBUG("update_token_counter end ");
}

static uint16_t copy_appIdHash_data(uint8_t *p_dest_buffer, uint8_t *p_apdu_data)
{
    // APDUからappIdHashを取得し
    // 指定の領域に格納
    uint8_t *p_appid_hash = p_apdu_data + U2F_CHAL_SIZE;
    memcpy(p_dest_buffer, p_appid_hash, U2F_APPID_SIZE);
    
    // コピーしたサイズを戻す
    return U2F_APPID_SIZE;
}

static uint16_t copy_challenge_data(uint8_t *p_dest_buffer, uint8_t *p_apdu_data)
{
    // APDUからchallengeを取得し
    // 指定の領域に格納
    uint8_t *p_challenge  = p_apdu_data;
    memcpy(p_dest_buffer, p_challenge, U2F_CHAL_SIZE);
    
    // コピーしたサイズを戻す
    return U2F_CHAL_SIZE;
}

static uint16_t copy_token_counter_data(uint8_t *p_dest_buffer, uint32_t token_counter)
{
    // トークンカウンター (4バイト) を
    // 指定の領域に格納
    p_dest_buffer[0] = token_counter >> 24 & 0xff;
    p_dest_buffer[1] = token_counter >> 16 & 0xff;
    p_dest_buffer[2] = token_counter >>  8 & 0xff;
    p_dest_buffer[3] = token_counter >>  0 & 0xff;
    
    // コピーしたサイズを戻す
    return 4;
}

static bool create_signature_base(ble_u2f_context_t *p_u2f_context, uint8_t user_presence, uint32_t token_counter)
{
    uint8_t offset = 0;

    // 署名ベースを格納する領域を確保
    if (ble_u2f_signature_data_allocate(p_u2f_context) == false) {
        return false;
    }

    // APDUからappIdHashを取得し格納
    uint8_t *signature_base_buffer = p_u2f_context->signature_data_buffer;
    FIDO_APDU_T *p_apdu = p_u2f_context->p_apdu;
    uint16_t copied_size = copy_appIdHash_data(signature_base_buffer + offset, p_apdu->data);
    offset += copied_size;

    // User Presence
    signature_base_buffer[offset++] = user_presence;

    // Counter
    copied_size = copy_token_counter_data(signature_base_buffer + offset, token_counter);
    offset += copied_size;
        
    // APDUからchallengeを取得し格納
    copied_size = copy_challenge_data(signature_base_buffer + offset, p_apdu->data);
    offset += copied_size;
    
    // メッセージのバイト数をセット
    p_u2f_context->signature_data_buffer_length = offset;

    return true;
}

static bool create_authentication_response_message(ble_u2f_context_t *p_u2f_context, uint8_t user_presence, uint32_t token_counter)
{
    // メッセージを格納する領域を確保
    // 確保した領域は、共有情報に設定します
    if (ble_u2f_response_message_allocate(p_u2f_context) == false) {
        return false;
    }
    
    // 確保した領域の先頭アドレスを取得
    uint8_t *response_message_buffer = p_u2f_context->response_message_buffer;
    uint16_t offset = 0;

    // user presence
    response_message_buffer[offset++] = user_presence;

    // token counter
    offset += copy_token_counter_data(response_message_buffer + offset, token_counter);

    // 署名格納領域からコピー
    memcpy(response_message_buffer + offset, 
        p_u2f_context->signature_data_buffer, 
        p_u2f_context->signature_data_buffer_length);
    offset += p_u2f_context->signature_data_buffer_length;

    if (p_u2f_context->p_apdu->Le < offset) {
        // Leを確認し、メッセージのバイト数がオーバーする場合
        // エラーレスポンスを送信するよう指示
        NRF_LOG_ERROR("Response message length(%d) exceeds Le(%d) ", offset, p_u2f_context->p_apdu->Le);
        p_u2f_context->p_ble_header->STATUS_WORD = U2F_SW_WRONG_LENGTH;
        return false;
    }

    // ステータスワード
    fido_set_status_word(response_message_buffer + offset, U2F_SW_NO_ERROR);
    offset += 2;
    
    // メッセージのバイト数をセット
    p_u2f_context->response_message_buffer_length = offset;

    return true;
}

static bool create_response_message(ble_u2f_context_t *p_u2f_context)
{
    // エラー時のレスポンスを「予期しないエラー」に設定
    p_u2f_context->p_ble_header->STATUS_WORD = 0x9504;
    
    // 署名ベースを生成（トークンカウンターは現在値＋１とする）
    uint8_t user_presence = p_u2f_context->user_presence_byte;
    uint32_t token_counter = p_u2f_context->token_counter + 1;
    if (create_signature_base(p_u2f_context, user_presence, token_counter) == false) {
        return false;
    }

    // キーハンドルから秘密鍵を取り出す(33バイト目以降)
    uint8_t *private_key_be = keyhandle_base_buffer + U2F_APPID_SIZE;

    // キーハンドルから取り出した秘密鍵により署名を生成
    if (ble_u2f_crypto_sign(private_key_be, p_u2f_context) != NRF_SUCCESS) {
        // 署名生成に失敗したら終了
        return false;
    }

    // ASN.1形式署名を格納する領域を準備
    if (ble_u2f_crypto_create_asn1_signature(p_u2f_context) == false) {
        // 生成された署名をASN.1形式署名に変換する
        // 変換失敗の場合終了
        return false;
    }

    if (create_authentication_response_message(p_u2f_context, user_presence, token_counter) == false) {
        // レスポンスメッセージ生成
        return false;
    }

    return true;
}

void ble_u2f_authenticate_resume_process(ble_u2f_context_t *p_u2f_context)
{
    // U2Fのリクエストデータを取得し、
    // レスポンス・メッセージを生成
    if (create_response_message(p_u2f_context) == false) {
        // NGであれば、エラーレスポンスを生成して戻す
        ble_u2f_send_error_response(p_u2f_context, p_u2f_context->p_ble_header->STATUS_WORD);
        return;
    }
    
    // appIdHashをキーとして、
    // トークンカウンターレコードを更新
    // (fds_record_update/writeまたはfds_gcが実行される)
    update_token_counter(p_u2f_context);
}

void ble_u2f_authenticate_do_process(ble_u2f_context_t *p_u2f_context)
{
    NRF_LOG_DEBUG("ble_u2f_authenticate start ");

    if (ble_u2f_flash_keydata_read(p_u2f_context) == false) {
        // 秘密鍵と証明書をFlash ROMから読込
        // NGであれば、エラーレスポンスを生成して戻す
        ble_u2f_send_error_response(p_u2f_context, 0x9501);
        return;
    }

    if (check_request_keyhandle(p_u2f_context) == false) {
        // リクエストデータのキーハンドルを復号化し、
        // リクエストデータのappIDHashがキーハンドルに含まれていない場合、
        // エラーレスポンスを生成して戻す
        NRF_LOG_ERROR("ble_u2f_authenticate: invalid keyhandle ");
        ble_u2f_send_error_response(p_u2f_context, U2F_SW_WRONG_DATA);
        return;
    }
    
    // appIdHashをリクエストデータから取得
    uint8_t *p_appid_hash = get_appid_from_apdu(p_u2f_context);
    if (ble_u2f_flash_token_counter_read(p_appid_hash) == false) {
        // appIdHashがトークンカウンターにない場合は
        // エラーレスポンスを生成して戻す
        NRF_LOG_ERROR("ble_u2f_authenticate: token counter not found ");
        ble_u2f_send_error_response(p_u2f_context, U2F_SW_WRONG_DATA);
        return;
    }

    // トークンカウンターを取得
    p_u2f_context->token_counter = ble_u2f_flash_token_counter_value();
    NRF_LOG_DEBUG("token counter value=%d ", p_u2f_context->token_counter);

    // control byte (P1) を参照
    uint8_t control_byte = p_u2f_context->p_apdu->P1;
    if (control_byte == 0x07) {
        // 0x07 ("check-only") の場合はここで終了し
        // SW_CONDITIONS_NOT_SATISFIED (0x6985)を戻す
        ble_u2f_send_error_response(p_u2f_context, U2F_SW_CONDITIONS_NOT_SATISFIED);
        return;
    }

    if (control_byte == 0x03) {
        // 0x03 ("enforce-user-presence-and-sign")
        // ユーザー所在確認が必要な場合は、ここで終了し
        // キープアライブ送信を開始する
        // ステータスバイトにTUP_NEEDED(0x02)を設定
        p_u2f_context->keepalive_status_byte = 0x02;
        ble_u2f_user_presence_verify_start(p_u2f_context);
        return;
    }

    // ユーザー所在確認不要の場合は、ここで
    // User presence byte(0x01)を生成し、
    // ただちにレスポンス・メッセージを生成
    p_u2f_context->user_presence_byte = 0x01;
    ble_u2f_authenticate_resume_process(p_u2f_context);
}

static void send_authentication_response(ble_u2f_context_t *p_u2f_context)
{
    // レスポンスを生成
    uint8_t command_for_response = p_u2f_context->p_ble_header->CMD;
    uint8_t *data_buffer = p_u2f_context->response_message_buffer;
    uint16_t data_buffer_length = p_u2f_context->response_message_buffer_length;

    // 生成したレスポンスを戻す
    ble_u2f_status_setup(command_for_response, data_buffer, data_buffer_length);
    ble_u2f_status_response_send(p_u2f_context->p_u2f);
}

void ble_u2f_authenticate_send_response(ble_u2f_context_t *p_u2f_context, fds_evt_t const *const p_evt)
{
    if (p_evt->result != FDS_SUCCESS) {
        // FDS処理でエラーが発生時は以降の処理を行わない
        ble_u2f_send_error_response(p_u2f_context, 0x9503);
        NRF_LOG_ERROR("ble_u2f_authenticate abend: FDS EVENT=%d ", p_evt->id);
        return;
    }

    if (p_evt->id == FDS_EVT_GC) {
        // FDSリソース不足解消のためGCが実行された場合は、
        // GC実行直前の処理を再実行
        NRF_LOG_WARNING("ble_u2f_authenticate retry: FDS GC done ");
        update_token_counter(p_u2f_context);

    } else if (p_evt->id == FDS_EVT_UPDATE || p_evt->id == FDS_EVT_WRITE) {
        // レスポンスを生成してU2Fクライアントに戻す
        send_authentication_response(p_u2f_context);
        NRF_LOG_DEBUG("ble_u2f_authenticate end ");
    }
}
