/* 
 * File:   demo_cdc_receive.c
 * Author: makmorit
 *
 * Created on 2019/10/16, 11:12
 */
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

void demo_cdc_receive_on_request_received(void)
{
    fido_log_debug("USB CDC recv [%s](%lu bytes)", m_cdc_buffer, m_cdc_buffer_size);
}
