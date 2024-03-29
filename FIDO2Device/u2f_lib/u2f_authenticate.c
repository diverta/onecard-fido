#include <stdio.h>
#include <string.h>

#include "fido_command_common.h"
#include "fido_common.h"
#include "u2f_define.h"
#include "u2f_signature.h"
#include "u2f_keyhandle.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(u2f_authenticate);
#endif

// ステータスワードを保持
static uint16_t status_word;

uint16_t u2f_authenticate_status_word(void)
{
    return status_word;
}

uint8_t *u2f_authenticate_get_appid(uint8_t *apdu_data)
{
    // appIdHashを参照し、APDUの33バイト目のアドレスを戻す
    uint8_t *p_appid_hash = apdu_data + U2F_CHAL_SIZE;
    return p_appid_hash;
}

bool u2f_authenticate_update_token_counter(uint8_t *p_appid_hash)
{
    // 開始ログを出力
    fido_log_debug("update_token_counter start ");

    // 現在のトークンカウンターを取得し、＋１する
    uint32_t token_counter = fido_command_sign_counter_value();
    token_counter++;

    // appIdHashをキーとして、
    // トークンカウンターレコードを更新する
    if (fido_command_sign_counter_update(p_appid_hash, token_counter) == false) {
        // NGであれば、エラーレスポンスを生成して終了
        return false;
    }

    // 後続のレスポンス生成・送信は、
    // Flash ROM書込み完了後に行われる
    fido_log_debug("update_token_counter end ");
    return true;
}

static uint16_t copy_appIdHash_data(uint8_t *p_dest_buffer, uint8_t *p_apdu_data)
{
    // APDUからappIdHashを取得し
    // 指定の領域に格納
    uint8_t *p_appid_hash = u2f_authenticate_get_appid(p_apdu_data);
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

static bool create_authenticate_signature_base(uint8_t *p_apdu_data, uint8_t user_presence, uint32_t token_counter)
{
    uint8_t offset = 0;

    // APDUからappIdHashを取得し格納
    uint8_t *signature_base_buffer = u2f_signature_data_buffer();
    uint16_t copied_size = copy_appIdHash_data(signature_base_buffer + offset, p_apdu_data);
    offset += copied_size;

    // User Presence
    signature_base_buffer[offset++] = user_presence;

    // Counter
    copied_size = copy_token_counter_data(signature_base_buffer + offset, token_counter);
    offset += copied_size;
        
    // APDUからchallengeを取得し格納
    copied_size = copy_challenge_data(signature_base_buffer + offset, p_apdu_data);
    offset += copied_size;
    
    // メッセージのバイト数をセット
    u2f_signature_base_data_size_set(offset);

    return true;
}

static bool create_authentication_response_message(uint8_t *response_message_buffer, size_t *response_length, uint32_t apdu_le, uint8_t user_presence, uint32_t token_counter)
{
    // メッセージを格納する領域を確保
    // 確保領域は0で初期化
    memset(response_message_buffer, 0, *response_length);
    uint16_t offset = 0;

    // user presence
    response_message_buffer[offset++] = user_presence;

    // token counter
    offset += copy_token_counter_data(response_message_buffer + offset, token_counter);

    // 署名格納領域からコピー
    memcpy(response_message_buffer + offset, 
        u2f_signature_data_buffer(), 
        u2f_signature_data_size());
    offset += u2f_signature_data_size();

    if (apdu_le < offset) {
        // Leを確認し、メッセージのバイト数がオーバーする場合
        // エラーレスポンスを送信するよう指示
        fido_log_error("Response message length(%d) exceeds Le(%d) ", offset, apdu_le);
        status_word = U2F_SW_WRONG_LENGTH;
        return false;
    }

    // ステータスワード
    fido_set_status_word(response_message_buffer + offset, U2F_SW_NO_ERROR);
    offset += 2;
    
    // メッセージのバイト数をセット
    *response_length = offset;

    return true;
}

bool u2f_authenticate_response_message(uint8_t *request_buffer, uint8_t *response_buffer, size_t *response_length, uint32_t apdu_le)
{
    // エラー時のレスポンスを「予期しないエラー」に設定
    status_word = 0x9504;

    // User presence byte(0x01)を生成
    uint8_t user_presence_byte = 0x01;

    // 現在のトークンカウンターを取得し、＋１する
    uint32_t token_counter = fido_command_sign_counter_value();
    token_counter++;
    
    // 署名ベースを生成
    if (create_authenticate_signature_base(request_buffer, user_presence_byte, token_counter) == false) {
        return false;
    }

    // サイト固有の秘密鍵を使用し、署名を生成
    if (fido_command_do_sign_with_keyhandle() == false) {
        return false;
    }

    if (create_authentication_response_message(response_buffer, response_length, apdu_le, user_presence_byte, token_counter) == false) {
        // レスポンスメッセージ生成
        return false;
    }

    return true;
}

bool u2f_authenticate_restore_keyhandle(uint8_t *apdu_data)
{
    // リクエストデータのキーハンドルを参照
    //   APDUの65バイト目以降
    uint32_t keyhandle_length = apdu_data[64];
    uint8_t *keyhandle_value = apdu_data + 65;

    // キーハンドルを復号化
    //   keyhandle_base_bufferに
    //   AppIDHash、秘密鍵が格納される
    u2f_keyhandle_restore(keyhandle_value, keyhandle_length);
    
    // リクエストデータからappIDHashを取得
    // キーハンドルに含まれているものと異なる場合はエラー
    uint8_t *p_appid_hash = u2f_authenticate_get_appid(apdu_data);
    if (strncmp((char *)u2f_keyhandle_base_buffer(), (char *)p_appid_hash, U2F_APPID_SIZE) != 0) {
        return false;
    }

    return true;
}
