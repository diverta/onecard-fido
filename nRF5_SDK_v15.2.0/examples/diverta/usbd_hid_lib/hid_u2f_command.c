/* 
 * File:   hid_u2f_command.c
 * Author: makmorit
 *
 * Created on 2018/11/21, 14:21
 */
#include "sdk_common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fds.h"

#include "u2f_flash.h"
#include "u2f_keyhandle.h"
#include "u2f_authenticate.h"
#include "u2f_register.h"
#include "hid_fido_command.h"
#include "hid_fido_receive.h"
#include "hid_fido_send.h"
#include "usbd_hid_common.h"

// for processing LED on/off
#include "fido_processing_led.h"

// for logging informations
#define NRF_LOG_MODULE_NAME hid_u2f_command
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for U2F_CHAL_SIZE
#include "u2f.h"

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
static uint8_t u2f_response_buffer[1024];
static size_t  u2f_response_length;

// 関数プロトタイプ
static void u2f_register_resume_process(void);
static void u2f_authenticate_resume_process(void);

static void u2f_resume_response_process(void)
{
    uint8_t ins;
    uint8_t cmd = hid_fido_receive_hid_header()->CMD;
    switch (cmd) {
        case U2F_COMMAND_MSG:
            // u2f_request_buffer の先頭バイトを参照
            //   [0]CLA [1]INS [2]P1 3[P2]
            ins = hid_fido_receive_apdu()->INS;
            if (ins == U2F_REGISTER) {
                NRF_LOG_INFO("U2F Register: completed the test of user presence");
                u2f_register_resume_process();
                
            } else if (ins == U2F_AUTHENTICATE) {
                NRF_LOG_INFO("U2F Authenticate: completed the test of user presence");
                u2f_authenticate_resume_process();
            }
            break;
        default:
            break;
    }
}


bool hid_u2f_command_on_mainsw_event(void)
{
    if (is_tup_needed) {
        // ユーザー所在確認が必要な場合
        // (＝ユーザーによるボタン押下が行われた場合)
        is_tup_needed = false;
        // 後続のレスポンス送信処理を実行
        u2f_resume_response_process();
        return true;
    }

    return false;
}

bool hid_u2f_command_on_mainsw_long_push_event(void)
{
    // NOP
    return true;
}

static void send_u2f_response(void)
{
    uint32_t cid = hid_fido_receive_hid_header()->CID;
    uint8_t cmd = hid_fido_receive_hid_header()->CMD;
    hid_fido_send_command_response(cid, cmd, u2f_response_buffer, u2f_response_length);
}

void hid_u2f_command_init(void)
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
    init_res.version_major = 1;
    init_res.version_minor = 1;
    init_res.version_build = 0;
    init_res.cflags        = 0;

    // レスポンスデータを転送
    uint32_t cid = hid_fido_receive_hid_header()->CID;
    uint8_t cmd = hid_fido_receive_hid_header()->CMD;
    hid_fido_send_command_response(cid, cmd, (uint8_t *)&init_res, sizeof(init_res));
}

void hid_u2f_command_version(void)
{
    // 編集領域を初期化
    memset(&version_res, 0x00, sizeof(version_res));

    // レスポンスデータを編集 (8 bytes)
    strcpy((char *)version_res.version, "U2F_V2");
    uint16_t status_word = U2F_SW_NO_ERROR;
    version_res.status_word[0] = (status_word >> 8) & 0x00ff;
    version_res.status_word[1] = status_word & 0x00ff;

    // レスポンスデータを転送
    uint32_t cid = hid_fido_receive_hid_header()->CID;
    uint8_t cmd = hid_fido_receive_hid_header()->CMD;
    hid_fido_send_command_response(cid, cmd, (uint8_t *)&version_res, sizeof(version_res));
}

static void send_u2f_hid_error_report(uint16_t status_word)
{
    // エラーステータスワードをビッグエンディアンで格納
    u2f_response_buffer[0] = (status_word >> 8) & 0x00ff;
    u2f_response_buffer[1] = (status_word >> 0) & 0x00ff;
    u2f_response_length = 2;
    
    // エラーステータスワードを送信
    send_u2f_response();
}

static uint8_t *get_appid_hash_from_u2f_request_apdu(void)
{
    // U2F Register／AuthenticateリクエストAPDUから
    // appid_hash(Application parameter)を取り出す
    uint8_t *apdu_data = hid_fido_receive_apdu()->data;
    uint8_t *p_appid_hash = apdu_data + U2F_CHAL_SIZE;
    
    return p_appid_hash;
}

static void u2f_register_do_process(void)
{
    // ユーザー所在確認フラグをクリア
    is_tup_needed = false;

    NRF_LOG_INFO("U2F Register start");

    if (u2f_flash_keydata_read() == false) {
        // 秘密鍵と証明書をFlash ROMから読込
        // NGであれば、エラーレスポンスを生成して戻す
        send_u2f_hid_error_report(0x9401);
        return;
    }

    if (u2f_flash_keydata_available() == false) {
        // 秘密鍵と証明書がFlash ROMに登録されていない場合
        // エラーレスポンスを生成して戻す
        send_u2f_hid_error_report(0x9402);
        return;
    }
    
    // control byte (P1) を参照
    uint8_t control_byte = hid_fido_receive_apdu()->P1;
    if (control_byte == 0x03) {
        // 0x03 ("enforce-user-presence-and-sign")
        // ユーザー所在確認が必要な場合は、ここで終了し
        // その旨のフラグを設定
        is_tup_needed = true;
        NRF_LOG_INFO("U2F Register: waiting to complete the test of user presence");
        // LED点滅を開始
        fido_processing_led_on(LED_FOR_USER_PRESENCE, LED_ON_OFF_INTERVAL_MSEC);
        return;
    }

    // ユーザー所在確認不要の場合は、後続のレスポンス送信処理を実行
    u2f_register_resume_process();
}

static void u2f_register_resume_process(void)
{
    // LEDを消灯させる
    fido_processing_led_off();

    // キーハンドルを新規生成
    uint8_t *p_appid_hash = get_appid_hash_from_u2f_request_apdu();
    u2f_register_generate_keyhandle(p_appid_hash);

    uint8_t *apdu_data = hid_fido_receive_apdu()->data;
    uint32_t apdu_le = hid_fido_receive_apdu()->Le;
    u2f_response_length = sizeof(u2f_response_buffer);
    if (u2f_register_response_message(apdu_data, u2f_response_buffer, &u2f_response_length, apdu_le) == false) {
        // U2Fのリクエストデータを取得し、
        // レスポンス・メッセージを生成
        // NGであれば、エラーレスポンスを生成して戻す
        send_u2f_hid_error_report(u2f_register_status_word());
        return;
    }

    // トークンカウンターレコードを追加
    // (fds_record_update/writeまたはfds_gcが実行される)
    if (u2f_register_add_token_counter(p_appid_hash) == false) {
        // 処理NGの場合、エラーレスポンスを生成して終了
        send_u2f_hid_error_report(0x9403);
    }
}

static void u2f_register_send_response(fds_evt_t const *const p_evt)
{
    if (p_evt->result != FDS_SUCCESS) {
        // FDS処理でエラーが発生時は以降の処理を行わない
        send_u2f_hid_error_report(0x9404);
        NRF_LOG_ERROR("U2F Register abend: FDS EVENT=%d ", p_evt->id);
        return;
    }

    if (p_evt->id == FDS_EVT_GC) {
        // FDSリソース不足解消のためGCが実行された場合は、
        // GC実行直前の処理を再実行
        NRF_LOG_WARNING("U2F Register retry: FDS GC done ");
        uint8_t *p_appid_hash = get_appid_hash_from_u2f_request_apdu();
        u2f_register_add_token_counter(p_appid_hash);

    } else if (p_evt->id == FDS_EVT_UPDATE || p_evt->id == FDS_EVT_WRITE) {
        // レスポンスを生成してU2Fクライアントに戻す
        send_u2f_response();
    }
}

static void u2f_authenticate_do_process(void)
{
    // ユーザー所在確認フラグをクリア
    is_tup_needed = false;
    
    NRF_LOG_INFO("U2F Authenticate start");

    if (u2f_flash_keydata_read() == false) {
        // 秘密鍵と証明書をFlash ROMから読込
        // NGであれば、エラーレスポンスを生成して戻す
        send_u2f_hid_error_report(0x9501);
        return;
    }

    uint8_t *apdu_data = hid_fido_receive_apdu()->data;
    if (u2f_authenticate_restore_keyhandle(apdu_data) == false) {
        // リクエストデータのキーハンドルを復号化し、
        // リクエストデータのappIDHashがキーハンドルに含まれていない場合、
        // エラーレスポンス(0x6A80)を生成して戻す
        NRF_LOG_ERROR("U2F Authenticate: invalid keyhandle ");
        send_u2f_hid_error_report(U2F_SW_WRONG_DATA);
        return;
    }

    // appIdHashをリクエストデータから取得し、
    // それに紐づくトークンカウンターを検索
    uint8_t *p_appid_hash = get_appid_hash_from_u2f_request_apdu();
    if (u2f_flash_token_counter_read(p_appid_hash) == false) {
        // appIdHashに紐づくトークンカウンターがない場合は
        // エラーレスポンスを生成して戻す
        NRF_LOG_ERROR("U2F Authenticate: token counter not found ");
        send_u2f_hid_error_report(U2F_SW_WRONG_DATA);
        return;
    }
    NRF_LOG_DEBUG("U2F Authenticate: token counter value=%d ", u2f_flash_token_counter_value());

    // control byte (P1) を参照
    uint8_t control_byte = hid_fido_receive_apdu()->P1;
    if (control_byte == 0x07) {
        // 0x07 ("check-only") の場合はここで終了し
        // SW_CONDITIONS_NOT_SATISFIED (0x6985)を戻す
        send_u2f_hid_error_report(U2F_SW_CONDITIONS_NOT_SATISFIED);
        return;
    }

    if (control_byte == 0x03) {
        // 0x03 ("enforce-user-presence-and-sign")
        // ユーザー所在確認が必要な場合は、ここで終了し
        // その旨のフラグを設定
        is_tup_needed = true;
        NRF_LOG_INFO("U2F Authenticate: waiting to complete the test of user presence");
        // LED点滅を開始
        fido_processing_led_on(LED_FOR_USER_PRESENCE, LED_ON_OFF_INTERVAL_MSEC);
        return;
    }

    // ユーザー所在確認不要の場合は、後続のレスポンス送信処理を実行
    u2f_authenticate_resume_process();
}

static void u2f_authenticate_resume_process(void)
{
    // LEDを消灯させる
    fido_processing_led_off();

    // U2Fのリクエストデータを取得し、
    // レスポンス・メッセージを生成
    uint8_t *apdu_data = hid_fido_receive_apdu()->data;
    uint32_t apdu_le = hid_fido_receive_apdu()->Le;
    u2f_response_length = sizeof(u2f_response_buffer);
    if (u2f_authenticate_response_message(apdu_data, u2f_response_buffer, &u2f_response_length, apdu_le) == false) {
        // U2Fのリクエストデータを取得し、
        // レスポンス・メッセージを生成
        // NGであれば、エラーレスポンスを生成して戻す
        send_u2f_hid_error_report(u2f_authenticate_status_word());
        return;
    }

    // appIdHashをキーとして、
    // トークンカウンターレコードを更新
    // (fds_record_update/writeまたはfds_gcが実行される)
    uint8_t *p_appid_hash = get_appid_hash_from_u2f_request_apdu();
    if (u2f_authenticate_update_token_counter(p_appid_hash) == false) {
        send_u2f_hid_error_report(0x9502);
    }
}

static void u2f_authenticate_send_response(fds_evt_t const *const p_evt)
{
    if (p_evt->result != FDS_SUCCESS) {
        // FDS処理でエラーが発生時は以降の処理を行わない
        send_u2f_hid_error_report(0x9503);
        NRF_LOG_ERROR("U2F Authenticate abend: FDS EVENT=%d ", p_evt->id);
        return;
    }

    if (p_evt->id == FDS_EVT_GC) {
        // FDSリソース不足解消のためGCが実行された場合は、
        // GC実行直前の処理を再実行
        NRF_LOG_WARNING("U2F Authenticate retry: FDS GC done ");
        uint8_t *p_appid_hash = get_appid_hash_from_u2f_request_apdu();
        u2f_authenticate_update_token_counter(p_appid_hash);

    } else if (p_evt->id == FDS_EVT_UPDATE || p_evt->id == FDS_EVT_WRITE) {
        // レスポンスを生成してU2Fクライアントに戻す
        send_u2f_response();
    }
}

void hid_u2f_command_msg(void)
{
    // u2f_request_buffer の先頭バイトを参照
    //   [0]CLA [1]INS [2]P1 3[P2]
    uint8_t ins = hid_fido_receive_apdu()->INS;
    if (ins == U2F_VERSION) {
        hid_u2f_command_version();

    } else if (ins == U2F_REGISTER) {
        u2f_register_do_process();

    } else if (ins == U2F_AUTHENTICATE) {
        u2f_authenticate_do_process();
    }
}

void hid_u2f_command_msg_send_response(fds_evt_t const *const p_evt)
{
    // u2f_request_buffer の先頭バイトを参照
    //   [0]CLA [1]INS [2]P1 3[P2]
    uint8_t ins = hid_fido_receive_apdu()->INS;
    if (ins == U2F_REGISTER) {
        u2f_register_send_response(p_evt);

    } else if (ins == U2F_AUTHENTICATE) {
        u2f_authenticate_send_response(p_evt);
    }
}

void hid_u2f_command_msg_report_sent(void)
{
    // u2f_request_buffer の先頭バイトを参照
    //   [0]CLA [1]INS [2]P1 3[P2]
    uint8_t ins = hid_fido_receive_apdu()->INS;
    if (ins == U2F_REGISTER) {
        NRF_LOG_INFO("U2F Register end");

    } else if (ins == U2F_AUTHENTICATE) {
        NRF_LOG_INFO("U2F Authenticate end");
    }
}
