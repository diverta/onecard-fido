#include "sdk_common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fds.h"

#include "u2f_flash.h"
#include "u2f_crypto_ecb.h"
#include "u2f_authenticate.h"
#include "u2f_register.h"
#include "usbd_hid_u2f.h"
#include "hid_u2f_common.h"
#include "hid_u2f_comm_interval_timer.h"
#include "hid_u2f_receive.h"
#include "hid_u2f_send.h"

// for ble_u2f_processing_led_on/off
#include "ble_u2f_processing_led.h"
#include "one_card_main.h"
#include "u2f_idling_led.h"

// for logging informations
#define NRF_LOG_MODULE_NAME hid_u2f_command
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for U2F_CHAL_SIZE
#include "u2f.h"

// ユーザー所在確認が必要かどうかを保持
static bool is_tup_needed = false;

//
// コマンド別のレスポンスデータ編集領域
//   固定長（64バイト）
//
typedef struct u2f_hid_init_response
{
    uint8_t nonce[8];
    uint8_t cid[4];
    uint8_t version_id;
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t version_build;
    uint8_t cflags;
    uint8_t filler[47];
} U2F_HID_INIT_RES;

typedef struct u2f_version_response
{
    uint8_t version[6];
    uint8_t status_word[2];
    uint8_t filler[56];
} U2F_VERSION_RES;

U2F_HID_INIT_RES  init_res;
U2F_VERSION_RES   version_res;

// 関数プロトタイプ
static void u2f_register_resume_process(void);
static void u2f_authenticate_resume_process(void);

static void u2f_resume_response_process(void)
{
    uint8_t ins;
    uint8_t cmd = hid_u2f_receive_hid_header()->CMD;
    switch (cmd) {
        case U2FHID_MSG:
            // u2f_request_buffer の先頭バイトを参照
            //   [0]CLA [1]INS [2]P1 3[P2]
            ins = hid_u2f_receive_apdu()->INS;
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
        // LEDを消灯させる
        ble_u2f_processing_led_off();
        // 後続のレスポンス送信処理を実行
        u2f_resume_response_process();
    }

    return true;
}

bool hid_u2f_command_on_mainsw_long_push_event(void)
{
    // NOP
    return true;
}

static void send_u2f_response(void)
{
    uint32_t cid = hid_u2f_receive_hid_header()->CID;
    uint8_t cmd = hid_u2f_receive_hid_header()->CMD;
    hid_u2f_send_setup(cid, cmd, u2f_response_buffer, u2f_response_length);
    hid_u2f_send_input_report();
}

static void u2f_hid_init_do_process(void)
{
    // 編集領域を初期化
    memset(&init_res, 0x00, sizeof(init_res));
    
    // nonce を取得
    uint8_t *nonce = hid_u2f_receive_apdu()->data;

    // レスポンスデータを編集 (17 bytes)
    //   CIDはインクリメントされたものを設定
    u2f_response_length = 17;
    memcpy(init_res.nonce, nonce, 8);
    set_CID(init_res.cid, get_incremented_CID());
    init_res.version_id    = 2;
    init_res.version_major = 1;
    init_res.version_minor = 1;
    init_res.version_build = 0;
    init_res.cflags        = 0;
    
    // レスポンスを格納
    memcpy(u2f_response_buffer, &init_res, u2f_response_length);

    // レスポンスを転送
    send_u2f_response();
}

static void u2f_version_do_process(void)
{
    // 編集領域を初期化
    memset(&version_res, 0x00, sizeof(version_res));

    // レスポンスデータを編集 (8 bytes)
    u2f_response_length = 8;
    strcpy((char *)version_res.version, "U2F_V2");
    uint16_t status_word = U2F_SW_NO_ERROR;
    version_res.status_word[0] = (status_word >> 8) & 0x00ff;
    version_res.status_word[1] = status_word & 0x00ff;
    
    // レスポンスを格納
    memcpy(u2f_response_buffer, &version_res, u2f_response_length);

    // レスポンスを転送
    send_u2f_response();
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
    uint8_t *apdu_data = hid_u2f_receive_apdu()->data;
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
    uint8_t control_byte = hid_u2f_receive_apdu()->P1;
    if (control_byte == 0x03) {
        // 0x03 ("enforce-user-presence-and-sign")
        // ユーザー所在確認が必要な場合は、ここで終了し
        // その旨のフラグを設定
        is_tup_needed = true;
        NRF_LOG_INFO("U2F Register: waiting to complete the test of user presence");
        // LED点滅を開始
        uint32_t led = one_card_get_U2F_context()->led_for_user_presence;
        ble_u2f_processing_led_on(led);
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

    uint8_t *apdu_data = hid_u2f_receive_apdu()->data;
    uint32_t apdu_le = hid_u2f_receive_apdu()->Le;
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

    uint8_t *apdu_data = hid_u2f_receive_apdu()->data;
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
    uint8_t control_byte = hid_u2f_receive_apdu()->P1;
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
        uint32_t led = one_card_get_U2F_context()->led_for_user_presence;
        ble_u2f_processing_led_on(led);
        return;
    }

    // ユーザー所在確認不要の場合は、後続のレスポンス送信処理を実行
    u2f_authenticate_resume_process();
}

static void u2f_authenticate_resume_process(void)
{
    // U2Fのリクエストデータを取得し、
    // レスポンス・メッセージを生成
    uint8_t *apdu_data = hid_u2f_receive_apdu()->data;
    uint32_t apdu_le = hid_u2f_receive_apdu()->Le;
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

static void send_error_command_response(uint8_t error_code) 
{
    // U2F ERRORコマンドに対応する
    // レスポンスデータを送信パケットに設定し送信
    generate_u2f_error_response(error_code);
    uint32_t cid = hid_u2f_receive_hid_header()->CID;
    hid_u2f_send_setup(cid, U2FHID_ERROR, u2f_response_buffer, u2f_response_length);
    hid_u2f_send_input_report();

    // アイドル時点滅処理を開始
    u2f_idling_led_on(one_card_get_U2F_context()->led_for_processing_fido);
}

//
// CBOR調査用サンプルプログラム
//
#include "cbor.h"

#define NUM_OF_CBOR_ELEMENTS 1
#define NUM_OF_VERSIONS      2
#define RESP_versions        0x01

static void ctap_get_versions(CborEncoder * encoder)
{
    int ret;
    CborEncoder array;
    CborEncoder map;

    ret = cbor_encoder_create_map(encoder, &map, NUM_OF_CBOR_ELEMENTS);
    if (ret == CborNoError) {
        ret = cbor_encode_uint(&map, RESP_versions);
        if (ret == CborNoError) {
            ret = cbor_encoder_create_array(&map, &array, NUM_OF_VERSIONS);
                ret = cbor_encode_text_stringz(&array, "U2F_V2");
                ret = cbor_encode_text_stringz(&array, "FIDO_2_0");
            ret = cbor_encoder_close_container(&map, &array);
        }
    }
    ret = cbor_encoder_close_container(encoder, &map);
    
    UNUSED_PARAMETER(ret);
}

static CborError ctap_verify(CborValue *it)
{
    while (!cbor_value_at_end(it)) {
        CborError err;
        CborType type = cbor_value_get_type(it);

        switch (type) {
        case CborArrayType:
        case CborMapType: {
            CborValue recursed;
            assert(cbor_value_is_container(it));
            err = cbor_value_enter_container(it, &recursed);
            if (err)
                return err;
            err = ctap_verify(&recursed);
            if (err)
                return err;
            err = cbor_value_leave_container(it, &recursed);
            if (err)
                return err;
            continue;
        }
        case CborIntegerType: {
            int64_t val;
            cbor_value_get_int64(it, &val);
            NRF_LOG_INFO("Integer value: %d", val);
            break;
        }
        case CborTextStringType: {
            char *buf;
            size_t n;
            err = cbor_value_dup_text_string(it, &buf, &n, it);
            if (err)
                return err;
            NRF_LOG_INFO("String value: %s", buf);
            free(buf);
            continue;
        }
        default:
            assert(false);
            break;
        }
        err = cbor_value_advance_fixed(it);
        if (err)
            return err;
    }
    return CborNoError;
}

static void test_cbor_encode(void)
{
    // 作業領域の初期化
    uint8_t encoded_buff[64];
    size_t  encoded_buff_size;
    encoded_buff_size = sizeof(encoded_buff);
    memset(encoded_buff, 0x00, encoded_buff_size);
    
    // CBORエンコードを実行する
    CborEncoder encoder;
    cbor_encoder_init(&encoder, encoded_buff, encoded_buff_size, 0);    
    ctap_get_versions(&encoder);
    NRF_LOG_INFO("===== cbor encoding test =====");
    NRF_LOG_HEXDUMP_INFO(encoded_buff, encoded_buff_size);

    // CBORデコードを実行する
    CborParser parser;
    CborValue it;
    cbor_parser_init(encoded_buff, encoded_buff_size, CborValidateCanonicalFormat, &parser, &it);
    NRF_LOG_INFO("===== cbor decoding test =====");
    ctap_verify(&it);

    NRF_LOG_INFO("===== cbor test end =====");
}

void hid_u2f_command_on_report_received(void)
{
    // for research
    test_cbor_encode();
    
    uint8_t  ins;
    NRF_LOG_INFO("CMD(0x%02x) LEN(%d)", 
        hid_u2f_receive_hid_header()->CMD, 
        hid_u2f_receive_hid_header()->LEN);

    // データ受信後に実行すべき処理を判定
    uint8_t cmd = hid_u2f_receive_hid_header()->CMD;
    switch (cmd) {
        case U2FHID_ERROR:
            send_error_command_response(hid_u2f_receive_hid_header()->ERROR);
            break;
            
        case U2FHID_INIT:
            u2f_hid_init_do_process();
            break;
            
        case U2FHID_MSG:
            // u2f_request_buffer の先頭バイトを参照
            //   [0]CLA [1]INS [2]P1 3[P2]
            ins = hid_u2f_receive_apdu()->INS;
            if (ins == U2F_VERSION) {
                u2f_version_do_process();
                
            } else if (ins == U2F_REGISTER) {
                u2f_register_do_process();
                
            } else if (ins == U2F_AUTHENTICATE) {
                u2f_authenticate_do_process();
            }
            break;
        default:
            break;
    }
}

void hid_u2f_command_on_fs_evt(fds_evt_t const *const p_evt)
{
    uint8_t  ins;

    // Flash ROM更新後に行われる後続処理を実行
    uint8_t cmd = hid_u2f_receive_hid_header()->CMD;
    switch (cmd) {
        case U2FHID_MSG:
            // u2f_request_buffer の先頭バイトを参照
            //   [0]CLA [1]INS [2]P1 3[P2]
            ins = hid_u2f_receive_apdu()->INS;
            if (ins == U2F_REGISTER) {
                u2f_register_send_response(p_evt);
                
            } else if (ins == U2F_AUTHENTICATE) {
                u2f_authenticate_send_response(p_evt);
            }
            break;
        default:
            break;
    }
}

void hid_u2f_command_on_report_sent(void)
{
    uint8_t  ins;

    // 全フレーム送信後に行われる後続処理を実行
    uint8_t cmd = hid_u2f_receive_hid_header()->CMD;
    switch (cmd) {
        case U2FHID_MSG:
            // u2f_request_buffer の先頭バイトを参照
            //   [0]CLA [1]INS [2]P1 3[P2]
            ins = hid_u2f_receive_apdu()->INS;
            if (ins == U2F_REGISTER) {
                NRF_LOG_INFO("U2F Register end");
                
            } else if (ins == U2F_AUTHENTICATE) {
                NRF_LOG_INFO("U2F Authenticate end");
            }
            break;
        default:
            break;
    }
}

void hid_u2f_command_on_process_started(void) 
{
    // 処理タイムアウト監視を開始
    hid_u2f_comm_interval_timer_start();

    // アイドル時点滅処理を停止
    u2f_idling_led_off(one_card_get_U2F_context()->led_for_processing_fido);
}

void hid_u2f_command_on_process_ended(void) 
{
    // 処理タイムアウト監視を停止
    hid_u2f_comm_interval_timer_stop();

    // アイドル時点滅処理を開始
    u2f_idling_led_on(one_card_get_U2F_context()->led_for_processing_fido);
}

void hid_u2f_command_on_process_timedout(void) 
{
    // USBポートにタイムアウトを通知する
    NRF_LOG_ERROR("USB HID communication timed out.");
    
    // コマンドをU2F ERRORに変更のうえ、
    // レスポンスデータを送信パケットに設定し送信
    send_error_command_response(0x7f);
}
