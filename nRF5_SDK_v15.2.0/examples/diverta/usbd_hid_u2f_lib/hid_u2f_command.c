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
    uint32_t cid = hid_u2f_receive_hid_header()->CID;
    uint8_t cmd = hid_u2f_receive_hid_header()->CMD;
    send_hid_input_report(cid, cmd, u2f_response_buffer, u2f_response_length);
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
    uint32_t cid = hid_u2f_receive_hid_header()->CID;
    uint8_t cmd = hid_u2f_receive_hid_header()->CMD;
    send_hid_input_report(cid, cmd, u2f_response_buffer, u2f_response_length);
}

static void u2f_register_do_process(void)
{
    // TODO: これは仮コードです。
}

static void u2f_authenticate_do_process(void)
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
    // Flash ROM更新後に行われる後続処理を実行
    UNUSED_PARAMETER(p_evt);
}
