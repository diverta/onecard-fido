/* 
 * File:   demo_cdc_receive.c
 * Author: makmorit
 *
 * Created on 2019/10/16, 11:12
 */
#include <stdio.h>
#include <string.h>

// プラットフォーム固有のインターフェース
#include "ble_service_central.h"
#include "ble_service_central_stat.h"
#include "usbd_service.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// CDC経由で連続して読み込まれた文字列を保持
// （1024文字分用意する。配列サイズは終端文字含む）
#define CDC_BUFFER_SIZE 1024
static char   m_cdc_buffer[CDC_BUFFER_SIZE + 1];
static size_t m_cdc_buffer_size;
static size_t m_cdc_buffer_size_received;

// CDC経由で応答する文字列を格納
static char cdc_response_buff[256];
static bool cdc_response_send = false;

void demo_cdc_receive_init(void)
{
    memset(m_cdc_buffer, 0, sizeof(m_cdc_buffer));
    m_cdc_buffer_size = 0;
    m_cdc_buffer_size_received = 0;
}

void demo_cdc_receive_char(char c)
{
    // 表示可能文字の場合はバッファにセット
    if (m_cdc_buffer_size_received < CDC_BUFFER_SIZE) {
        m_cdc_buffer[m_cdc_buffer_size_received] = c;
        m_cdc_buffer_size_received++;
    }
}

void demo_cdc_receive_char_terminate(void)
{
    if (m_cdc_buffer_size_received > 0) {
        m_cdc_buffer_size = m_cdc_buffer_size_received;
        m_cdc_buffer[m_cdc_buffer_size] = 0;
        m_cdc_buffer_size_received = 0;

    } else {
        demo_cdc_receive_init();
    }
}

//
// デモ機能（RSSIログ出力）
//
static void get_rssi_log_output(void)
{
    fido_log_debug("get_rssi_log_output called.");

    sprintf(cdc_response_buff, "get_rssi_log_output called.\r\n");
    cdc_response_send = true;
}

static void get_rssi_log_event(void)
{
    if (usbd_cdc_port_is_open()) {
        // 仮想COMポートに接続されている場合は、
        // ログを出力する
        get_rssi_log_output();

    } else {
        // 仮想COMポートから切断されている場合は、
        // スキャンを停止し、タイマーも停止させる
        fido_repeat_process_timer_stop();
    }
}

static void get_rssi_log(void)
{
    // １秒ごとに get_rssi_log_event を呼出
    fido_repeat_process_timer_start(1000, get_rssi_log_event);
}

//
// デモ機能（One Cardスキャン）
//
static void resume_function_after_scan(void)
{
    // 作業領域初期化
    memset(cdc_response_buff, 0, sizeof(cdc_response_buff));

    // 統計情報をデバッグ出力
    ble_service_central_stat_debug_print();

    //
    // One cardデバイスが見つかったらプリントして終了
    // （422E0000-E141-11E5-A837-0800200C9A66）
    // 複数スキャンされた場合は、最もRSSI値が大きいものが戻ります。
    //
    char *one_card_uuid_string = "422E0000-E141-11E5-A837-0800200C9A66";
    ADV_STAT_INFO_T *info = ble_service_central_stat_match_uuid(one_card_uuid_string);
    if (info == NULL) {
        sprintf(cdc_response_buff, "One card peripheral device not found.\r\n");
    } else {
        sprintf(cdc_response_buff, "One card peripheral device found (NAME=%s, ADDR=%s)\r\n", 
            info->dev_name, ble_service_central_stat_btaddr_string(info->peer_addr));
    }
    cdc_response_send = true;
}

static void onecard_scan_demo(void)
{
    // One Cardデバイスをスキャンし、
    // 見つかった場合、ログをプリント
    ble_service_central_scan_start(1000, resume_function_after_scan);
}

//
// CDCからデータ受信時の処理
//
void demo_cdc_receive_on_request_received(void)
{
    fido_log_debug("USB CDC recv [%s](%lu bytes)", m_cdc_buffer, m_cdc_buffer_size);

    // デモ機能用のコマンドを解析
    if (strcmp(m_cdc_buffer, "get_rssi_log") == 0) {
        get_rssi_log();

    } else if (strcmp(m_cdc_buffer, "onecard_scan_demo") == 0) {
        onecard_scan_demo();
    }
}

//
// CDCへデータ送信時の処理
//   CDC送信処理は、エコーバック処理と重複を避けるため、
//   mainループ（usbd_service_do_process）から呼出させるよう実装
//
bool demo_cdc_send_response_ready(void)
{
    // 応答文字列有り／無しを示すフラグを戻す
    return cdc_response_send;
}

char *demo_cdc_send_response_buffer_get(void)
{
    // フラグをリセットし、応答文字列を格納している領域の参照を戻す
    cdc_response_send = false;
    return cdc_response_buff;
}
