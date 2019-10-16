/* 
 * File:   demo_cdc_receive.c
 * Author: makmorit
 *
 * Created on 2019/10/16, 11:12
 */
#include <string.h>

// プラットフォーム固有のインターフェース
#include "ble_service_central.h"
#include "ble_service_central_stat.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// 連続して読み込まれた文字列を保持
// （1024文字分用意する。配列サイズは終端文字含む）
#define CDC_BUFFER_SIZE 1024
static char   m_cdc_buffer[CDC_BUFFER_SIZE + 1];
static size_t m_cdc_buffer_size;
static size_t m_cdc_buffer_size_received;

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
// デモ機能
//
static void resume_function_after_scan(void)
{
    // 統計情報をデバッグ出力
    ble_service_central_stat_debug_print();

    //
    // One cardダミーデバイスが見つかったらプリントして終了
    // （422E0000-E141-11E5-A837-0800200C9A66）
    //
    char *one_card_uuid_string = "422E0000-E141-11E5-A837-0800200C9A66";
    ADV_STAT_INFO_T *info = ble_service_central_stat_match_uuid(one_card_uuid_string);
    if (info == NULL) {
        fido_log_debug("onecard_scan_demo: One card peripheral device not found.");
    } else {
        fido_log_debug("onecard_scan_demo: One card peripheral device found (NAME=%s, ADDR=%s)", 
            info->dev_name, ble_service_central_stat_btaddr_string(info->peer_addr));
    }
}

static void onecard_scan_demo(void)
{
    // BLEダミーデバイスをスキャンし、
    // 見つかった場合、ログをプリント
    ble_service_central_scan_start(1000, resume_function_after_scan);
}

void demo_cdc_receive_on_request_received(void)
{
    fido_log_debug("USB CDC recv [%s](%lu bytes)", m_cdc_buffer, m_cdc_buffer_size);

    // デモ機能用のコマンドを解析
    if (strcmp(m_cdc_buffer, "onecard_scan_demo") == 0) {
        onecard_scan_demo();
    }
}
