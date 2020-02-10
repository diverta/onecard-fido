#include <stdio.h>
#include <string.h>

#include "fido_command_common.h"
#include "fido_common.h"
#include "u2f.h"
#include "u2f_signature.h"
#include "u2f_keyhandle.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// ステータスワードを保持
static uint16_t status_word;

uint16_t u2f_register_status_word(void)
{
    return status_word;
}

bool u2f_register_add_token_counter(uint8_t *p_appid_hash)
{
    // 開始ログを出力
    fido_log_debug("add_token_counter start ");

    // appIdHashをキーとして、
    // トークンカウンターレコードを追加する
    //   appIdHashの開始位置は
    //   APDUの33バイト目から末尾までの32バイト
    //   counterの値は0とする
    uint32_t token_counter = 0;
    if (fido_flash_token_counter_write(p_appid_hash, token_counter, p_appid_hash) == false) {
        return false;
    }

    // 後続のレスポンス生成・送信は、
    // Flash ROM書込み完了後に行われる
    fido_log_debug("add_token_counter end ");
    return true;
}

static uint16_t copy_apdu_data(uint8_t *p_dest_buffer, uint8_t *p_apdu_data)
{
    // APDUからappIdHash, challengeを取得し
    // 指定の領域に格納
    uint8_t *p_appid_hash = p_apdu_data + U2F_CHAL_SIZE;
    memcpy(p_dest_buffer, p_appid_hash, U2F_APPID_SIZE);

    uint8_t *p_challenge  = p_apdu_data;
    memcpy(p_dest_buffer + U2F_APPID_SIZE, p_challenge, U2F_CHAL_SIZE);

    // コピーしたサイズを戻す
    return U2F_APPID_SIZE + U2F_CHAL_SIZE;
}

static uint16_t copy_publickey_data(uint8_t *p_dest_buffer)
{
    // 公開鍵は public_key_raw_data に
    // ビッグエンディアンで格納される
    uint8_t *p_publickey = fido_command_keypair_public_key();
    uint16_t copied_size = 0;

    // 1バイト目＝0x04
    p_dest_buffer[copied_size++] = U2F_POINT_UNCOMPRESSED;

    // 2-33バイト目＝公開鍵のx部を設定
    for (uint8_t i = 0; i < 32; i++) {
        p_dest_buffer[copied_size++] = *(p_publickey + i);
    }

    // 34-65バイト目＝公開鍵のy部を設定
    for (uint8_t i = 32; i < 64; i++) {
        p_dest_buffer[copied_size++] = *(p_publickey + i);
    }

    // コピーしたサイズを戻す
    return copied_size;
}

static bool create_register_signature_base(uint8_t *p_apdu)
{
    uint8_t offset = 0;

    // RFU
    uint8_t *signature_base_buffer = u2f_signature_data_buffer();
    signature_base_buffer[offset++] = 0x00;

    // APDUからappIdHash, challengeを取得し格納
    uint16_t copied_size = copy_apdu_data(signature_base_buffer + offset, p_apdu);
    offset += copied_size;

    // キーハンドルを格納
    copied_size = u2f_keyhandle_buffer_size();
    memcpy(signature_base_buffer + offset, u2f_keyhandle_buffer(), copied_size);
    offset += copied_size;

    // 公開鍵を格納
    copied_size = copy_publickey_data(signature_base_buffer + offset);
    offset += copied_size;

    // メッセージのバイト数をセット
    u2f_signature_base_data_size_set(offset);

    return true;
}

static bool create_registration_response_message(uint8_t *response_message_buffer, size_t *response_length, uint32_t apdu_le)
{
    // メッセージを格納する領域を確保
    // 確保領域は0で初期化
    memset(response_message_buffer, 0, *response_length);
    uint16_t offset = 0;

    // reserved(0x05)
    response_message_buffer[offset++] = 0x05;

    // 公開鍵
    offset += copy_publickey_data(response_message_buffer + offset);

    // キーハンドル長
    uint8_t keyhandle_length = u2f_keyhandle_buffer_size();
    response_message_buffer[offset++] = keyhandle_length;

    // キーハンドル
    memcpy(response_message_buffer + offset, u2f_keyhandle_buffer(), keyhandle_length);
    offset += keyhandle_length;

    // 証明書格納領域と長さを取得
    uint8_t *cert_buffer = fido_flash_cert_data();
    uint32_t cert_buffer_length = fido_flash_cert_data_length();

    // 証明書格納領域からコピー
    memcpy(response_message_buffer + offset, cert_buffer, cert_buffer_length);
    offset += cert_buffer_length;

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

bool u2f_register_response_message(uint8_t *request_buffer, uint8_t *response_buffer, size_t *response_length, uint32_t apdu_le)
{
    // エラー時のレスポンスを「予期しないエラー」に設定
    status_word = 0x9405;
    
    // 署名ベースを生成
    if (create_register_signature_base(request_buffer) == false) {
        return false;
    }

    // 認証器固有の秘密鍵を使用して署名生成
    if (fido_command_do_sign_with_privkey() == false) {
        return false;
    }

    if (create_registration_response_message(response_buffer, response_length, apdu_le) == false) {
        // レスポンスメッセージ生成
        return false;
    }
    return true;
}

void u2f_register_generate_keyhandle(uint8_t *p_appid_hash)
{
    // APDUから取得したappIdHashを使用し、
    // キーハンドルを新規生成する
    u2f_keyhandle_generate(p_appid_hash);
}
