#include "sdk_common.h"
#if NRF_MODULE_ENABLED(BLE_U2F)
#include <stdio.h>
#include <string.h>
#include "ble_u2f_securekey.h"
#include "ble_u2f_crypto.h"
#include "ble_u2f_flash.h"
#include "ble_u2f_status.h"
#include "ble_u2f_crypto_ecb.h"
#include "ble_u2f_util.h"

// for nrf_value_length_t
#include "nrf_crypto_types.h"

// for logging informations
#define NRF_LOG_MODULE_NAME "ble_u2f_register"
#include "nrf_log.h"


static bool add_token_counter(ble_u2f_context_t *p_u2f_context)
{
    // appIdHashをキーとして、
    // トークンカウンターレコードを追加する
    //   appIdHashの開始位置は
    //   APDUの33バイト目から末尾までの32バイト
    //   counterの値は0とする
    U2F_APDU_T *p_apdu = p_u2f_context->p_apdu;
    uint8_t *p_appid_hash = p_apdu->data + U2F_CHAL_SIZE;
    uint32_t token_counter = 0;
    uint32_t reserve_word = 0xffffffff;
    if (ble_u2f_flash_token_counter_write(p_u2f_context, p_appid_hash, token_counter, reserve_word) == false) {
        return false;
    }

    // 後続のレスポンス生成・送信は、
    // Flash ROM書込み完了後に行われる
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
    // 公開鍵は、ワード単位でリトルエンディアン格納されているので
    // ビッグエンディアンに変換してから格納
    nrf_value_length_t *public_key = ble_u2f_crypto_public_key();
    uint8_t *p_publickey = public_key->p_value;
    uint16_t copied_size = 0;

    // 1バイト目＝0x04
    p_dest_buffer[copied_size++] = U2F_POINT_UNCOMPRESSED;

    // 2-33バイト目＝公開鍵のx部を設定
    for (uint8_t i = 32; i > 0; i--) {
        p_dest_buffer[copied_size++] = *(p_publickey + i - 1);
    }

    // 34-65バイト目＝公開鍵のy部を設定
    for (uint8_t i = 64; i > 32; i--) {
        p_dest_buffer[copied_size++] = *(p_publickey + i - 1);
    }

    // コピーしたサイズを戻す
    return copied_size;
}

static bool create_register_signature_base(ble_u2f_context_t *p_u2f_context)
{
    uint8_t offset = 0;

    // 署名ベースを格納する領域を確保
    if (ble_u2f_signature_data_allocate(p_u2f_context) == false) {
        return false;
    }

    // RFU
    uint8_t *signature_base_buffer = p_u2f_context->signature_data_buffer;
    signature_base_buffer[offset++] = 0x00;

    // APDUからappIdHash, challengeを取得し格納
    U2F_APDU_T *p_apdu = p_u2f_context->p_apdu;
    uint16_t copied_size = copy_apdu_data(signature_base_buffer + offset, p_apdu->data);
    offset += copied_size;

    // キーハンドルを格納
    copied_size = sizeof(keyhandle_buffer);
    memcpy(signature_base_buffer + offset, keyhandle_buffer, copied_size);
    offset += copied_size;

    // 公開鍵を格納
    copied_size = copy_publickey_data(signature_base_buffer + offset);
    offset += copied_size;

    // メッセージのバイト数をセット
    p_u2f_context->signature_data_buffer_length = offset;

    return true;
}

static bool create_registration_response_message(ble_u2f_context_t *p_u2f_context, nrf_value_length_t *p_signature)
{
    // メッセージを格納する領域を確保
    // 確保した領域は、共有情報に設定します
    if (ble_u2f_response_message_allocate(p_u2f_context) == false) {
        return false;
    }

    // 確保した領域の先頭アドレスを取得
    uint8_t *response_message_buffer = p_u2f_context->response_message_buffer;
    uint16_t offset = 0;

    // reserved(0x05)
    response_message_buffer[offset++] = 0x05;

    // 公開鍵
    offset += copy_publickey_data(response_message_buffer + offset);

    // キーハンドル長
    uint8_t keyhandle_length = sizeof(keyhandle_buffer);
    response_message_buffer[offset++] = keyhandle_length;

    // キーハンドル
    memcpy(response_message_buffer + offset, keyhandle_buffer, keyhandle_length);
    offset += keyhandle_length;

    // 証明書格納領域と長さを取得
    uint8_t *cert_buffer = ble_u2f_securekey_cert(p_u2f_context);
    uint32_t cert_buffer_length = ble_u2f_securekey_cert_length(p_u2f_context);

    // 証明書格納領域からコピー
    memcpy(response_message_buffer + offset, cert_buffer, cert_buffer_length);
    offset += cert_buffer_length;

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

static bool create_register_response_message(ble_u2f_context_t *p_u2f_context)
{
    // 署名ベースを生成
    if (create_register_signature_base(p_u2f_context) == false) {
        return false;
    }

    // 署名用の秘密鍵を取得し、署名を生成
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

    if (create_registration_response_message(p_u2f_context, &signature) == false) {
        // レスポンスメッセージ生成
        return false;
    }
    return true;
}

static void generate_keyhandle(ble_u2f_context_t *p_u2f_context)
{
    // micro-eccにより、キーペアを新規生成する
    ble_u2f_crypto_generate_keypair();
    nrf_value_length_t *private_key = ble_u2f_crypto_private_key();

    // APDUから取得したappIdHash、秘密鍵を使用し、
    // キーハンドルを新規生成する
    uint8_t *p_appid_hash = p_u2f_context->p_apdu->data + U2F_CHAL_SIZE;
    ble_u2f_crypto_ecb_generate_keyhandle(p_appid_hash, private_key);
}

void ble_u2f_register_do_process(ble_u2f_context_t *p_u2f_context)
{
    NRF_LOG_DEBUG("ble_u2f_register start \r\n");

    if (ble_u2f_flash_keydata_read(p_u2f_context) == false) {
        // 秘密鍵と証明書をFlash ROMから読込
        // NGであれば、エラーレスポンスを生成して戻す
        ble_u2f_send_error_response(p_u2f_context, 0x01);
        return;
    }

    if (ble_u2f_flash_keydata_available(p_u2f_context) == false) {
        // 秘密鍵と証明書がFlash ROMに登録されていない場合
        // エラーレスポンスを生成して戻す
        ble_u2f_send_error_response(p_u2f_context, 0x02);
        return;
    }
    
    // キーハンドルを新規生成
    generate_keyhandle(p_u2f_context);

    if (create_register_response_message(p_u2f_context) == false) {
        // U2Fのリクエストデータを取得し、
        // レスポンス・メッセージを生成
        // NGであれば、エラーレスポンスを生成して戻す
        ble_u2f_send_error_response(p_u2f_context, p_u2f_context->p_ble_header->STATUS_WORD);
        return;
    }

    if (add_token_counter(p_u2f_context) == false) {
        // トークンカウンターレコードを追加し、
        // (fds_record_update/writeまたはfds_gcが実行される)
        // 処理NGの場合、エラーレスポンスを生成して戻す
        ble_u2f_send_error_response(p_u2f_context, 0x03);
        return;
    }
}

static void send_register_response(ble_u2f_context_t *p_u2f_context)
{
    // レスポンスを生成
    uint8_t command_for_response = p_u2f_context->p_ble_header->CMD;
    uint8_t *data_buffer = p_u2f_context->response_message_buffer;
    uint16_t data_buffer_length = p_u2f_context->response_message_buffer_length;

    // 生成したレスポンスを戻す
    ble_u2f_status_setup(command_for_response, data_buffer, data_buffer_length);
    ble_u2f_status_response_send(p_u2f_context->p_u2f);
}

void ble_u2f_register_send_response(ble_u2f_context_t *p_u2f_context, fds_evt_t const *const p_evt)
{
    if (p_evt->result != FDS_SUCCESS) {
        // FDS処理でエラーが発生時は以降の処理を行わない
        ble_u2f_send_error_response(p_u2f_context, 0x04);
        NRF_LOG_ERROR("ble_u2f_register abend: FDS EVENT=%d \r\n", p_evt->id);
        return;
    }

    if (p_evt->id == FDS_EVT_GC) {
        // FDSリソース不足解消のためGCが実行された場合は、
        // GC実行直前の処理を再実行
        NRF_LOG_WARNING("ble_u2f_register retry: FDS GC done \r\n");
        ble_u2f_register_do_process(p_u2f_context);

    } else if (p_evt->id == FDS_EVT_UPDATE || p_evt->id == FDS_EVT_WRITE) {
        // レスポンスを生成してU2Fクライアントに戻す
        send_register_response(p_u2f_context);
        NRF_LOG_DEBUG("ble_u2f_register end \r\n");
    }
}

#endif // NRF_MODULE_ENABLED(BLE_U2F)
