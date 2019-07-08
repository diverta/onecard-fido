/* 
 * File:   fido_u2f_command.c
 * Author: makmorit
 *
 * Created on 2018/11/21, 14:21
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//
// プラットフォーム非依存コード
//
#include "u2f.h"
#include "u2f_authenticate.h"
#include "u2f_keyhandle.h"
#include "u2f_register.h"
#include "fido_common.h"
#include "fido_hid_channel.h"
#include "fido_hid_command.h"
#include "fido_hid_receive.h"
#include "fido_hid_send.h"
#include "fido_ble_command.h"
#include "fido_ble_receive.h"
#include "fido_ble_send.h"
#include "fido_nfc_receive.h"
#include "fido_nfc_send.h"
#include "u2f.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// トランスポート種別を保持
static TRANSPORT_TYPE m_transport_type;

// ユーザー所在確認が必要かどうかを保持
static bool is_tup_needed = false;

//
// U2F_VERSIONコマンドのレスポンスデータ編集領域
//   固定長（8バイト）
//
typedef struct {
    uint8_t version[6];
    uint8_t status_word[2];
} U2F_VERSION_RES_T;

//
// U2Fレスポンスデータ格納領域
//
static HID_INIT_RES_T init_res;
static U2F_VERSION_RES_T version_res;
static uint8_t response_buffer[1024];
static size_t  response_length;

// 関数プロトタイプ
static void u2f_register_resume_process(void);
static void u2f_authenticate_resume_process(void);

static uint8_t get_u2f_command_byte(void)
{
    uint8_t cmd;
    switch (m_transport_type) {
        case TRANSPORT_HID:
            cmd = fido_hid_receive_header()->CMD;
            break;
        case TRANSPORT_BLE:
            cmd = fido_ble_receive_header()->CMD;
            break;
        case TRANSPORT_NFC:
            // ヘッダーが存在しないため、
            // U2Fコマンドは常に U2F_COMMAND_MSG になります
            cmd = U2F_COMMAND_MSG;
            break;
        default:
            cmd = 0x00;
            break;
    }

    return cmd;
}

static FIDO_APDU_T *get_receive_apdu(void)
{
    FIDO_APDU_T *p_apdu;
    switch (m_transport_type) {
        case TRANSPORT_HID:
            p_apdu = fido_hid_receive_apdu();
            break;
        case TRANSPORT_BLE:
            p_apdu = fido_ble_receive_apdu();
            break;
        case TRANSPORT_NFC:
            p_apdu = fido_nfc_receive_apdu();
            break;
        default:
            p_apdu = NULL;
            break;
    }

    return p_apdu;
}

static uint8_t get_u2f_command_ins_byte(void)
{
    if (get_u2f_command_byte() != U2F_COMMAND_MSG) {
        // U2Fコマンド以外はINS未設定とみなす
        return 0;
    }

    if (get_receive_apdu() == NULL) {
        // トランスポート未設定時はINSも未設定とみなす
        return 0;
    }

    // u2f_request_buffer の先頭バイトを参照
    //   [0]CLA [1]INS [2]P1 3[P2]
    return get_receive_apdu()->INS;
}

static void u2f_resume_response_process(void)
{
    uint8_t ins;
    switch (get_u2f_command_byte()) {
        case U2F_COMMAND_MSG:
            // u2f_request_buffer の先頭バイトを参照
            //   [0]CLA [1]INS [2]P1 3[P2]
            ins = get_receive_apdu()->INS;
            if (ins == U2F_REGISTER) {
                fido_log_info("U2F Register: completed the test of user presence");
                u2f_register_resume_process();
                
            } else if (ins == U2F_AUTHENTICATE) {
                fido_log_info("U2F Authenticate: completed the test of user presence");
                u2f_authenticate_resume_process();
            }
            break;
        default:
            break;
    }
}


bool fido_u2f_command_on_mainsw_event(void)
{
    if (is_tup_needed) {
        // ユーザー所在確認が必要な場合
        // (＝ユーザーによるボタン押下が行われた場合)
        is_tup_needed = false;
        // キープアライブを停止
        fido_user_presence_verify_end();
        // 後続のレスポンス送信処理を実行
        u2f_resume_response_process();
        return true;
    }

    return false;
}

bool fido_u2f_command_on_mainsw_long_push_event(void)
{
    // NOP
    return true;
}

void fido_u2f_command_keepalive_timer_handler(void)
{
    if (is_tup_needed) {
        // キープアライブ・コマンドを実行する（BLE限定機能）
        if (m_transport_type == TRANSPORT_BLE) {
            // TUP_NEEDED: 0x02
            fido_ble_send_command_response_no_callback(U2F_COMMAND_KEEPALIVE, 0x02);
        }
    }
}

void fido_u2f_command_send_response(uint8_t *response, size_t length)
{
    // レスポンスデータを送信パケットに設定し送信
    if (m_transport_type == TRANSPORT_HID) {
        uint32_t cid = fido_hid_receive_header()->CID;
        uint8_t cmd = fido_hid_receive_header()->CMD;
        fido_hid_send_command_response(cid, cmd, response, length);

    } else if (m_transport_type == TRANSPORT_BLE) {
        uint8_t cmd = fido_ble_receive_header()->CMD;
        fido_ble_send_command_response(cmd, response, length);

    } else if (m_transport_type == TRANSPORT_NFC) {
        fido_nfc_send_command_response(response, length);
    } 
}

void fido_u2f_command_hid_init(void)
{
    // 編集領域を初期化
    memset(&init_res, 0x00, sizeof(init_res));

    // nonce を取得
    uint8_t *nonce = fido_hid_receive_apdu()->data;

    // レスポンスデータを編集 (17 bytes)
    //   CIDはインクリメントされたものを設定
    memcpy(init_res.nonce, nonce, 8);
    set_CID(init_res.cid, get_incremented_CID());
    init_res.version_id    = 2;
    init_res.version_major = 1;
    init_res.version_minor = 1;
    init_res.version_build = 0;
    init_res.cflags        = 0;

    // レスポンスデータを転送
    uint32_t cid = fido_hid_receive_header()->CID;
    uint8_t cmd = fido_hid_receive_header()->CMD;
    fido_hid_send_command_response(cid, cmd, (uint8_t *)&init_res, sizeof(init_res));
    fido_log_info("U2F HID init done");
}

static void u2f_command_version(void)
{
    // 編集領域を初期化
    memset(&version_res, 0x00, sizeof(version_res));

    // レスポンスデータを編集 (8 bytes)
    strcpy((char *)version_res.version, U2F_V2_VERSION_STRING);
    uint16_t status_word = U2F_SW_NO_ERROR;
    version_res.status_word[0] = (status_word >> 8) & 0x00ff;
    version_res.status_word[1] = status_word & 0x00ff;

    // レスポンスデータを転送
    fido_u2f_command_send_response((uint8_t *)&version_res, sizeof(version_res));
    fido_log_info("U2F Version done");
}

static void send_u2f_error_status_response(uint16_t status_word)
{
    // エラーステータスワードをビッグエンディアンで格納
    response_buffer[0] = (status_word >> 8) & 0x00ff;
    response_buffer[1] = (status_word >> 0) & 0x00ff;
    response_length = 2;
    
    // エラーステータスワードを送信
    fido_u2f_command_send_response(response_buffer, response_length);
}

static uint8_t *get_appid_hash_from_u2f_request_apdu(void)
{
    // U2F Register／AuthenticateリクエストAPDUから
    // appid_hash(Application parameter)を取り出す
    uint8_t *apdu_data = get_receive_apdu()->data;
    uint8_t *p_appid_hash = apdu_data + U2F_CHAL_SIZE;
    
    return p_appid_hash;
}

static void u2f_command_register(void)
{
    // ユーザー所在確認フラグをクリア
    is_tup_needed = false;

    fido_log_info("U2F Register start");

    if (fido_flash_skey_cert_read() == false) {
        // 秘密鍵と証明書をFlash ROMから読込
        // NGであれば、エラーレスポンスを生成して戻す
        send_u2f_error_status_response(0x9401);
        return;
    }

    if (fido_flash_skey_cert_available() == false) {
        // 秘密鍵と証明書がFlash ROMに登録されていない場合
        // エラーレスポンスを生成して戻す
        send_u2f_error_status_response(0x9402);
        return;
    }
    
    // control byte (P1) を参照
    uint8_t control_byte = get_receive_apdu()->P1;
    if (control_byte == 0x03) {
        // 0x03 ("enforce-user-presence-and-sign")
        // ユーザー所在確認が必要な場合は、ここで終了し
        // その旨のフラグを設定
        is_tup_needed = true;
        fido_log_info("U2F Register: waiting to complete the test of user presence");
        // LED点滅を開始
        fido_user_presence_verify_start(U2F_KEEPALIVE_INTERVAL_MSEC);
        return;
    }

    // ユーザー所在確認不要の場合は、後続のレスポンス送信処理を実行
    u2f_register_resume_process();
}

static void u2f_register_resume_process(void)
{
    // キーハンドルを新規生成
    uint8_t *p_appid_hash = get_appid_hash_from_u2f_request_apdu();
    u2f_register_generate_keyhandle(p_appid_hash);

    uint8_t *apdu_data = get_receive_apdu()->data;
    uint32_t apdu_le = get_receive_apdu()->Le;
    response_length = sizeof(response_buffer);
    if (u2f_register_response_message(apdu_data, response_buffer, &response_length, apdu_le) == false) {
        // U2Fのリクエストデータを取得し、
        // レスポンス・メッセージを生成
        // NGであれば、エラーレスポンスを生成して戻す
        send_u2f_error_status_response(u2f_register_status_word());
        return;
    }

    // トークンカウンターレコードを追加
    // (fds_record_update/writeまたはfds_gcが実行される)
    if (u2f_register_add_token_counter(p_appid_hash) == false) {
        // 処理NGの場合、エラーレスポンスを生成して終了
        send_u2f_error_status_response(0x9403);
    }
}

static void u2f_register_send_response(fido_flash_event_t const *const p_evt)
{
    if (p_evt->write_update && p_evt->token_counter_write) {
        // レスポンスを生成してU2Fクライアントに戻す
        fido_u2f_command_send_response(response_buffer, response_length);
    }
}

static void u2f_command_authenticate(void)
{
    // ユーザー所在確認フラグをクリア
    is_tup_needed = false;
    
    fido_log_info("U2F Authenticate start");

    if (fido_flash_skey_cert_read() == false) {
        // 秘密鍵と証明書をFlash ROMから読込
        // NGであれば、エラーレスポンスを生成して戻す
        send_u2f_error_status_response(0x9501);
        return;
    }

    uint8_t *apdu_data = get_receive_apdu()->data;
    if (u2f_authenticate_restore_keyhandle(apdu_data) == false) {
        // リクエストデータのキーハンドルを復号化し、
        // リクエストデータのappIDHashがキーハンドルに含まれていない場合、
        // エラーレスポンス(0x6A80)を生成して戻す
        fido_log_error("U2F Authenticate: invalid keyhandle ");
        send_u2f_error_status_response(U2F_SW_WRONG_DATA);
        return;
    }

    // appIdHashをリクエストデータから取得し、
    // それに紐づくトークンカウンターを検索
    uint8_t *p_appid_hash = get_appid_hash_from_u2f_request_apdu();
    if (fido_flash_token_counter_read(p_appid_hash) == false) {
        // appIdHashに紐づくトークンカウンターがない場合は
        // エラーレスポンスを生成して戻す
        fido_log_error("U2F Authenticate: token counter not found ");
        send_u2f_error_status_response(U2F_SW_WRONG_DATA);
        return;
    }
    fido_log_debug("U2F Authenticate: token counter value=%d ", fido_flash_token_counter_value());

    // control byte (P1) を参照
    uint8_t control_byte = get_receive_apdu()->P1;
    if (control_byte == 0x07) {
        // 0x07 ("check-only") の場合はここで終了し
        // SW_CONDITIONS_NOT_SATISFIED (0x6985)を戻す
        send_u2f_error_status_response(U2F_SW_CONDITIONS_NOT_SATISFIED);
        return;
    }

    if (control_byte == 0x03) {
        // 0x03 ("enforce-user-presence-and-sign")
        // ユーザー所在確認が必要な場合は、ここで終了し
        // その旨のフラグを設定
        is_tup_needed = true;
        fido_log_info("U2F Authenticate: waiting to complete the test of user presence");
        // LED点滅を開始
        fido_user_presence_verify_start(U2F_KEEPALIVE_INTERVAL_MSEC);
        return;
    }

    // ユーザー所在確認不要の場合は、後続のレスポンス送信処理を実行
    u2f_authenticate_resume_process();
}

static void u2f_authenticate_resume_process(void)
{
    // U2Fのリクエストデータを取得し、
    // レスポンス・メッセージを生成
    uint8_t *apdu_data = get_receive_apdu()->data;
    uint32_t apdu_le = get_receive_apdu()->Le;
    response_length = sizeof(response_buffer);
    if (u2f_authenticate_response_message(apdu_data, response_buffer, &response_length, apdu_le) == false) {
        // U2Fのリクエストデータを取得し、
        // レスポンス・メッセージを生成
        // NGであれば、エラーレスポンスを生成して戻す
        send_u2f_error_status_response(u2f_authenticate_status_word());
        return;
    }

    // appIdHashをキーとして、
    // トークンカウンターレコードを更新
    // (fds_record_update/writeまたはfds_gcが実行される)
    uint8_t *p_appid_hash = get_appid_hash_from_u2f_request_apdu();
    if (u2f_authenticate_update_token_counter(p_appid_hash) == false) {
        send_u2f_error_status_response(0x9502);
    }
}

static void u2f_authenticate_send_response(fido_flash_event_t const *const p_evt)
{
    if (p_evt->write_update && p_evt->token_counter_write) {
        // レスポンスを生成してU2Fクライアントに戻す
        fido_u2f_command_send_response(response_buffer, response_length);
    }
}

void fido_u2f_command_msg(TRANSPORT_TYPE transport_type)
{
    // トランスポート種別を保持
    m_transport_type = transport_type;

    // u2f_request_buffer の先頭バイトを参照
    //   [0]CLA [1]INS [2]P1 3[P2]
    uint8_t ins = get_u2f_command_ins_byte();
    switch (ins) {
        case U2F_REGISTER:
            u2f_command_register();
            break;
        case U2F_AUTHENTICATE:
            u2f_command_authenticate();
            break;
        case U2F_VERSION:
            u2f_command_version();
            break;
        default:
            // INSが不正の場合は終了
            fido_log_debug("Invalid INS(0x%02x) ", ins);
            fido_ble_send_status_word(get_u2f_command_byte(), U2F_SW_INS_NOT_SUPPORTED);
            break;
    }
}

void fido_u2f_command_msg_send_response(fido_flash_event_t *const p_evt)
{
    // u2f_request_buffer の先頭バイトを参照
    //   [0]CLA [1]INS [2]P1 3[P2]
    uint8_t ins = get_u2f_command_ins_byte();
    if (ins == U2F_REGISTER) {
        u2f_register_send_response(p_evt);

    } else if (ins == U2F_AUTHENTICATE) {
        u2f_authenticate_send_response(p_evt);
    }
}

void fido_u2f_command_msg_report_sent(void)
{
    // u2f_request_buffer の先頭バイトを参照
    //   [0]CLA [1]INS [2]P1 3[P2]
    uint8_t ins = get_u2f_command_ins_byte();
    if (ins == U2F_REGISTER) {
        fido_log_info("U2F Register end");

    } else if (ins == U2F_AUTHENTICATE) {
        fido_log_info("U2F Authenticate end");
    }
}

void fido_u2f_command_ping(TRANSPORT_TYPE transport_type)
{
    // トランスポート種別を保持
    m_transport_type = transport_type;

    // PINGの場合は
    // リクエストのヘッダーとデータを編集せず
    // レスポンスとして戻す（エコーバック）
    fido_u2f_command_send_response(get_receive_apdu()->data, get_receive_apdu()->data_length);
    fido_log_info("U2F Ping done");
}

void fido_u2f_command_flash_failed(void)
{
    // Flash ROM処理でエラーが発生時はエラーレスポンス送信
    uint8_t ins = get_u2f_command_ins_byte();
    if (ins == U2F_REGISTER) {
        send_u2f_error_status_response(0x9404);
        fido_log_error("U2F Register abend");

    } else if (ins == U2F_AUTHENTICATE) {
        send_u2f_error_status_response(0x9503);
        fido_log_error("U2F Authenticate abend");
    }
}

void fido_u2f_command_flash_gc_done(void)
{
    // for nRF52840:
    // FDSリソース不足解消のためGCが実行された場合は、
    // GC実行直前の処理を再実行
    uint8_t ins = get_u2f_command_ins_byte();
    if (ins == U2F_REGISTER) {
        fido_log_warning("U2F Register retry: FDS GC done ");
        u2f_register_add_token_counter(get_appid_hash_from_u2f_request_apdu());

    } else if (ins == U2F_AUTHENTICATE) {
        fido_log_warning("U2F Authenticate retry: FDS GC done ");
        u2f_authenticate_update_token_counter(get_appid_hash_from_u2f_request_apdu());
    }
}
