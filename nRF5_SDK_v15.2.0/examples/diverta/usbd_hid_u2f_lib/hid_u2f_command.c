#include "sdk_common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fds.h"

#include "u2f_flash.h"
#include "u2f_crypto_ecb.h"
#include "u2f_register.h"
#include "usbd_hid_u2f.h"
#include "hid_u2f_common.h"
#include "hid_u2f_receive.h"
#include "hid_u2f_send.h"

// for logging informations
#define NRF_LOG_MODULE_NAME hid_u2f_command
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for U2F_CHAL_SIZE
#include "u2f.h"

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

bool hid_u2f_command_on_mainsw_event(void)
{
    // NOP
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

static uint8_t *get_appid_hash_from_register_apdu(void)
{
    // U2F RegisterリクエストAPDUから
    // appid_hash を取り出す
    uint8_t *apdu_data = hid_u2f_receive_apdu()->data;
    uint8_t *p_appid_hash = apdu_data + U2F_CHAL_SIZE;
    
    return p_appid_hash;
}

static void u2f_register_do_process(void)
{
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
    
    // キーハンドルを新規生成
    uint8_t *p_appid_hash = get_appid_hash_from_register_apdu();
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
        NRF_LOG_ERROR("u2f_register abend: FDS EVENT=%d ", p_evt->id);
        return;
    }

    if (p_evt->id == FDS_EVT_GC) {
        // FDSリソース不足解消のためGCが実行された場合は、
        // GC実行直前の処理を再実行
        NRF_LOG_WARNING("u2f_register retry: FDS GC done ");
        uint8_t *p_appid_hash = get_appid_hash_from_register_apdu();
        u2f_register_add_token_counter(p_appid_hash);

    } else if (p_evt->id == FDS_EVT_UPDATE || p_evt->id == FDS_EVT_WRITE) {
        // レスポンスを生成してU2Fクライアントに戻す
        send_u2f_response();
        NRF_LOG_DEBUG("u2f_register end ");
    }
}

static void u2f_authenticate_do_process(void)
{
    // TODO: これは仮コードです。
}

static void u2f_authenticate_send_response(fds_evt_t const *const p_evt)
{
    // TODO: これは仮コードです。
}

void hid_u2f_command_on_report_received(void)
{
    uint8_t  ins;

    // データ受信後に実行すべき処理を判定
    uint8_t cmd = hid_u2f_receive_hid_header()->CMD;
    switch (cmd) {
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
