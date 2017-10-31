#include "sdk_common.h"
#if NRF_MODULE_ENABLED(BLE_U2F)
#include <stdio.h>
#include <string.h>
#include "ble_u2f_securekey.h"
#include "ble_u2f_crypto.h"
#include "ble_u2f_flash.h"
#include "ble_u2f_status.h"
#include "ble_u2f_command.h"
#include "ble_u2f_user_presence.h"
#include "ble_u2f_util.h"

// for logging informations
#define NRF_LOG_MODULE_NAME "ble_u2f_authenticate"
#include "nrf_log.h"


static bool check_request_keyhandle(ble_u2f_context_t *p_u2f_context)
{
    // リクエストデータのキーハンドルを参照
    //   APDUの65バイト目以降
    U2F_APDU_T *p_apdu = p_u2f_context->p_apdu;
    uint8_t request_keyhandle_length = p_apdu->data[64];
    char *p_request_keyhandle = (char *)p_apdu->data + 65;

    // トークン内で保持されているキーハンドルを参照
    uint8_t stored_keyhandle_length = 4;
    char *p_stored_keyhandle = (char *)(p_u2f_context->securekey_buffer + SKEY_WORD_NUM);

    if (request_keyhandle_length != stored_keyhandle_length) {
        // 両方のキーハンドルを比較し、長さが異なる場合はNGと判定
        return false;

    } else if (strncmp(p_request_keyhandle, p_stored_keyhandle, stored_keyhandle_length) != 0) {
        // 両方のキーハンドルを比較し、内容が異なっている場合はNGと判定
        return false;
    }

    return true;
}

static uint8_t *get_appid_from_apdu(ble_u2f_context_t *p_u2f_context)
{
    // appIdHashを参照し、APDUの33バイト目のアドレスを戻す
    U2F_APDU_T *p_apdu = p_u2f_context->p_apdu;
    uint8_t *p_appid_hash = p_apdu->data + 32;

    return p_appid_hash;
}

static bool update_token_counter(ble_u2f_context_t *p_u2f_context)
{
    // appIdHash、トークンカウンターを
    // 共有情報から取得
    uint8_t *p_appid_hash = get_appid_from_apdu(p_u2f_context);
    uint32_t token_counter = p_u2f_context->token_counter;

    // appIdHashをキーとして、
    // トークンカウンターレコードを更新する
    uint32_t reserve_word = 0xffffffff;
    if (ble_u2f_flash_token_counter_write(p_u2f_context, p_appid_hash, token_counter, reserve_word) == false) {
        return false;
    }

    // 後続のレスポンス生成・送信は、
    // Flash ROM書込み完了後に行われる
    return true;
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

static bool create_signature_base(ble_u2f_context_t *p_u2f_context, uint32_t *keypair_cert_buffer, uint8_t user_presence, uint32_t token_counter)
{
    uint8_t offset = 0;

    // 署名ベースを格納する領域を確保
    if (ble_u2f_signature_data_allocate(p_u2f_context) == false) {
        return false;
    }

    // APDUからappIdHashを取得し格納
    uint8_t *signature_base_buffer = p_u2f_context->signature_data_buffer;
    U2F_APDU_T *p_apdu = p_u2f_context->p_apdu;
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

static bool create_authentication_response_message(ble_u2f_context_t *p_u2f_context, nrf_value_length_t *p_signature, uint8_t user_presence, uint32_t token_counter)
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
    memcpy(response_message_buffer + offset, p_signature->p_value, p_signature->length);
    offset += p_signature->length;

    if (p_u2f_context->p_apdu->Le < offset) {
        // Leを確認し、メッセージのバイト数がオーバーする場合
        // エラーレスポンスを送信するよう指示
        NRF_LOG_ERROR("Response message length(%d) exceeds Le(%d) \r\n", offset, p_u2f_context->p_apdu->Le);
        p_u2f_context->p_ble_header->STATUS_WORD = U2F_SW_WRONG_LENGTH;
        return false;
    }

    // ステータスワード
    ble_u2f_set_status_word(response_message_buffer + offset, U2F_SW_NO_ERROR);
    offset += 2;
    
    // メッセージのバイト数をセット
    p_u2f_context->response_message_buffer_length = offset;

    return true;
}

static bool create_response_message(ble_u2f_context_t *p_u2f_context)
{
    // 署名ベースを生成
    uint8_t user_presence = p_u2f_context->user_presence_byte;
    uint32_t token_counter = p_u2f_context->token_counter;
    uint32_t *keypair_cert_buffer = p_u2f_context->securekey_buffer;
    if (create_signature_base(p_u2f_context, keypair_cert_buffer, user_presence, token_counter) == false) {
        return false;
    }

    // キーペアから署名を生成
    uint8_t *signature_base_buffer = p_u2f_context->signature_data_buffer;
    uint16_t signature_base_buffer_length = p_u2f_context->signature_data_buffer_length;
    if (ble_u2f_crypto_sign(ble_u2f_securekey_skey(p_u2f_context), 
        signature_base_buffer, signature_base_buffer_length) != NRF_SUCCESS) {
        // 署名生成に失敗したら終了
        return false;
    }
    
    // ASN.1形式署名を格納する領域を準備
    nrf_value_length_t signature;
    signature.p_value = p_u2f_context->signature_data_buffer;
    if (ble_u2f_crypto_create_asn1_signature(&signature) == false) {
        // 生成された署名をASN.1形式署名に変換する
        // 変換失敗の場合終了
        return false;
    }

    if (create_authentication_response_message(p_u2f_context, &signature, user_presence, token_counter) == false) {
        // レスポンスメッセージ生成
        return false;
    }
    return true;
}

void ble_u2f_authenticate_resume_process(ble_u2f_context_t *p_u2f_context)
{
    p_u2f_context->token_counter++;
    if (create_response_message(p_u2f_context) == false) {
        // U2Fのリクエストデータを取得し、
        // レスポンス・メッセージを生成
        // NGであれば、エラーレスポンスを生成して戻す
        ble_u2f_send_error_response(p_u2f_context, p_u2f_context->p_ble_header->STATUS_WORD);
        return;
    }
    
    // appIdHashをキーとして、
    // トークンカウンターレコードを更新する
    if (update_token_counter(p_u2f_context) == false) {
        // エラーレスポンスを生成して戻す
        ble_u2f_send_error_response(p_u2f_context, 0x03);
        return;
    }
}

void ble_u2f_authenticate_do_process(ble_u2f_context_t *p_u2f_context)
{
    NRF_LOG_DEBUG("ble_u2f_authenticate start \r\n");

    if (ble_u2f_flash_keydata_read(p_u2f_context) == false) {
        // キーペア（秘密鍵／公開鍵／キーハンドル）および
        // 証明書をFlash ROMから読込
        // NGであれば、エラーレスポンスを生成して戻す
        ble_u2f_send_error_response(p_u2f_context, 0x01);
        return;
    }

    if (check_request_keyhandle(p_u2f_context) == false) {
        // リクエストデータのキーハンドルを参照し、
        // トークン内で保持されているキーハンドルと違う場合、
        // エラーレスポンスを生成して戻す
        NRF_LOG_ERROR("ble_u2f_authenticate: invalid keyhandle \r\n");
        ble_u2f_send_error_response(p_u2f_context, U2F_SW_WRONG_DATA);
        return;
    }
    
    // appIdHashをリクエストデータから取得
    uint8_t *p_appid_hash = get_appid_from_apdu(p_u2f_context);
    if (ble_u2f_flash_token_counter_read(p_appid_hash) == false) {
        // appIdHashがトークンカウンターにない場合は
        // エラーレスポンスを生成して戻す
        NRF_LOG_ERROR("ble_u2f_authenticate: token counter not found \r\n");
        ble_u2f_send_error_response(p_u2f_context, U2F_SW_WRONG_DATA);
        return;
    }

    // トークンカウンターを取得
    p_u2f_context->token_counter = ble_u2f_flash_token_counter_value();
    NRF_LOG_DEBUG("token counter value=%d \r\n", p_u2f_context->token_counter);

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
    if (p_evt->id != FDS_EVT_WRITE && p_evt->id != FDS_EVT_UPDATE) {
        // write/update完了イベントでない場合はスルー
        return;
    }

    ret_code_t result = p_evt->result;
    if (result == FDS_SUCCESS) {
        // レスポンスを生成してU2Fクライアントに戻す
        send_authentication_response(p_u2f_context);
        NRF_LOG_DEBUG("ble_u2f_authenticate end \r\n");

    } else {
        // エラーレスポンスを生成してU2Fクライアントに戻す
        ble_u2f_send_error_response(p_u2f_context, 0x05);
        NRF_LOG_ERROR("ble_u2f_authenticate abend \r\n");
    }
}

#endif // NRF_MODULE_ENABLED(BLE_U2F)
