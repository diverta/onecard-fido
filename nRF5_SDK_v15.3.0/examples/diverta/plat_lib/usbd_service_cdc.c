/* 
 * File:   usbd_service_cdc.c
 * Author: makmorit
 *
 * Created on 2020/04/27, 10:27
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "app_error.h"

#include "usbd_service.h"
#include "demo_cdc_service.h"

// for logging informations
#define NRF_LOG_MODULE_NAME usbd_service_cdc
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for debug cdc buffer
#define NRF_LOG_HEXDUMP_DEBUG_REPORT false

//
// CDC関連
//
#include "app_usbd_cdc_acm.h"

// CDCが使用するバッファ
static char m_rx_buffer[1];

// 仮想COMポートからデータを受信したことを示すフラグ
static bool m_cdc_buffer_received = false;

// 仮想COMポート接続中を示すフラグ
static bool m_cdc_port_open = false;

// エコーバック用バッファ（128byte分用意する）
#define CDC_ECHO_BUFFER_SIZE 128
static char    m_echo_buff[CDC_ECHO_BUFFER_SIZE];
static uint8_t m_echo_idx = 0;

static void echo_char_init(void)
{
    memset(m_echo_buff, 0, sizeof(m_echo_buff));
    m_echo_idx = 0;
}

static void echo_char_add(char c)
{
    if (m_echo_idx < CDC_ECHO_BUFFER_SIZE) {
        m_echo_buff[m_echo_idx++] = c;
    }
}

static ret_code_t usbd_cdc_char_print(app_usbd_cdc_acm_t const *p_cdc_acm, void *buff, size_t size)
{
    ret_code_t ret = NRF_SUCCESS;
    for (size_t i = 0; i < size; i++) {
        do {
            ret = app_usbd_cdc_acm_write(p_cdc_acm, buff + i, 1);
        } while (ret == NRF_ERROR_BUSY);
    }
    return ret;
}

static void echo_char_print(app_usbd_cdc_acm_t const *p_cdc_acm)
{
    // １文字づつ出力（BUSYの場合はリトライ）
    usbd_cdc_char_print(p_cdc_acm, m_echo_buff, m_echo_idx);

#if NRF_LOG_HEXDUMP_DEBUG_REPORT
    NRF_LOG_DEBUG("echo_char_print(%u bytes) %d", m_echo_idx);
    NRF_LOG_HEXDUMP_DEBUG(m_echo_buff, m_echo_idx);
#endif

    // エコーバッファを初期化
    echo_char_init();
}

static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_cdc_acm_user_event_t event);

APP_USBD_CDC_ACM_GLOBAL_DEF(m_app_cdc_acm,
                            cdc_acm_user_ev_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_AT_V250
);

static void usbd_cdc_port_open(app_usbd_class_inst_t const *p_inst)
{
    // Setup first transfer
    app_usbd_cdc_acm_t const *p_cdc_acm = app_usbd_cdc_acm_class_get(p_inst);
    app_usbd_cdc_acm_read(p_cdc_acm, m_rx_buffer, 1);

    // 作業領域を初期化
    demo_cdc_receive_init();
    echo_char_init();

    // 仮想COMポートが接続されている状態
    m_cdc_port_open = true;
    NRF_LOG_DEBUG("USB CDC port opened");

    // 仮想COMポートに接続された時の処理を実行
    demo_cdc_event_connected();
}

static void usbd_cdc_port_close(app_usbd_class_inst_t const *p_inst)
{
    // 仮想COMポートから切断されている状態
    (void)p_inst;
    m_cdc_port_open = false;
    NRF_LOG_DEBUG("USB CDC port closed");

    // 仮想COMポートから切断された時の処理を実行
    demo_cdc_event_disconnected();
}

static void usbd_cdc_buffer_char_add(char c)
{
    // 表示可能文字の場合はバッファにセット
    demo_cdc_receive_char(c);

    // echo back
    echo_char_add(c);
}

static void usbd_cdc_buffer_set(void)
{
    // バッファ設定を完了
    demo_cdc_receive_char_terminate();

    // echo back
    echo_char_add('\r');
    echo_char_add('\n');
}

static void usbd_cdc_buffer_read(app_usbd_class_inst_t const *p_inst)
{
    app_usbd_cdc_acm_t const *p_cdc_acm = app_usbd_cdc_acm_class_get(p_inst);
    bool ctrl_char = false;
    bool input_cr = false;
    
    do {
        if (input_cr) {
            // 直前の文字がCR文字だった場合
            // 入力データに改行が含まれていると判断し、
            // バッファに復帰・改行文字を追加
            usbd_cdc_buffer_char_add('\r');
            usbd_cdc_buffer_char_add('\n');
            input_cr = false;
        }
        
        // 読み込んだ文字に対して処理を行う
        char c = m_rx_buffer[0];
        if (c == '\r') {
            // 入力データに含めるかどうかは後判定のため、
            // ここではフラグを設定するのみ
            input_cr = true;

        } else if (c < 32 || c > 126 || ctrl_char) {
            // 表示不能文字が出現した場合は以降の文字を空読み
            ctrl_char = true;
        
        } else {
            // 表示可能文字の場合はバッファに文字を追加
            usbd_cdc_buffer_char_add(c);
        }

    // 続けて１文字分読込み
    // バッファ内に入力された場合は、ループを続行
    } while (app_usbd_cdc_acm_read(p_cdc_acm, m_rx_buffer, 1) == NRF_SUCCESS);
    
    if (input_cr) {
        // 最終文字としてCR文字が入力された場合は、
        // データ終端とみなし、データ入力を完了
        usbd_cdc_buffer_set();
    }
    
    // echo back
    echo_char_print(p_cdc_acm);
    if (input_cr) {
        m_cdc_buffer_received = true;
    }
}

static void usbd_cdc_buffer_write(app_usbd_class_inst_t const *p_inst)
{
    (void)p_inst;
}

static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const *p_inst,
                                    app_usbd_cdc_acm_user_event_t event)
{
    switch (event) {
        case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
            usbd_cdc_port_open(p_inst);
            break;
        case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
            usbd_cdc_port_close(p_inst);
            break;
        case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
            usbd_cdc_buffer_write(p_inst);
            break;
        case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
            usbd_cdc_buffer_read(p_inst);
            break;
        default:
            break;
    }
}

static bool usbd_cdc_frame_send(uint8_t *buffer_for_send, size_t size)
{
    // 指定された領域から出力
    ret_code_t ret = usbd_cdc_char_print(&m_app_cdc_acm, buffer_for_send, size);
    if (ret != NRF_SUCCESS) {
        // USBポートには装着されているが、
        // PCのターミナルアプリケーションが受信していない場合
        // NRF_LOG_ERROR("Log text is not received by terminal app");
        return false;
    }
    return true;
}

void usbd_cdc_init(void)
{
    app_usbd_class_inst_t const * class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&m_app_cdc_acm);
    ret_code_t ret = app_usbd_class_append(class_cdc_acm);
    if (ret != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_usbd_class_append(class_cdc_acm) returns 0x%02x ", ret);
    }
    APP_ERROR_CHECK(ret);

    NRF_LOG_DEBUG("usbd_cdc_init() done");
}

bool usbd_cdc_port_is_open(void)
{
    // 仮想COMポート接続中の場合 true
    return m_cdc_port_open;
}

void usbd_service_cdc_do_process(void)
{
    // 仮想COMポート接続中の場合は
    // HIDサービスを稼働させない
    if (m_cdc_buffer_received) {
        // CDCサービスから受信データがあった場合、
        // デモ機能を実行
        m_cdc_buffer_received = false;
        demo_cdc_receive_on_request_received();

    } else if (demo_cdc_send_response_ready()) {
        // CDCサービスからレスポンスデータがあった場合、
        // CDCへ出力
        char *b = demo_cdc_send_response_buffer_get();
        usbd_cdc_frame_send((uint8_t *)b, strlen(b));
    }
}
