/* 
 * File:   usbd_cdc_service.c
 * Author: makmorit
 *
 * Created on 2019/03/05, 11:21
 */
#include "sdk_common.h"

#include "nrf_drv_usbd.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_power.h"

#include "app_error.h"
#include "app_util.h"
#include "app_usbd_core.h"
#include "app_usbd.h"
#include "app_usbd_string_desc.h"
#include "app_usbd_cdc_acm.h"
#include "app_usbd_serial_num.h"

// for logging informations
#define NRF_LOG_MODULE_NAME usbd_cdc_service
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for logger main process
#include "usbd_cdc_logger_process.h"

/**
 * @brief Enable USB power detection
 */
#ifndef USBD_POWER_DETECTION
#define USBD_POWER_DETECTION true
#endif

static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_cdc_acm_user_event_t event);

#define CDC_ACM_COMM_INTERFACE  0
#define CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN2

#define CDC_ACM_DATA_INTERFACE  1
#define CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN1
#define CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT1

/**
 * @brief CDC_ACM class instance
 * */
APP_USBD_CDC_ACM_GLOBAL_DEF(m_app_cdc_acm,
                            cdc_acm_user_ev_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_AT_V250
);

// CDCが使用するバッファ
static char m_rx_buffer[NRF_DRV_USBD_EPSIZE];
static char m_tx_buffer[NRF_DRV_USBD_EPSIZE];

// 連続して読み込まれた文字列を保持
static char   m_line_buffer[NRF_DRV_USBD_EPSIZE];
static size_t m_line_size;

/**
 * @brief User event handler @ref app_usbd_cdc_acm_user_ev_handler_t (headphones)
 * */
static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_cdc_acm_user_event_t event)
{
    ret_code_t ret;
    size_t     size;
    static size_t received;
    app_usbd_cdc_acm_t const *p_cdc_acm = app_usbd_cdc_acm_class_get(p_inst);

    switch (event) {
        case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
            // Setup first transfer
            ret = app_usbd_cdc_acm_read(&m_app_cdc_acm, m_rx_buffer, 1);
            UNUSED_VARIABLE(ret);
            // 作業領域を初期化
            m_line_size = 0;
            received = 0;
            break;
        case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
            break;
        case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
            break;
        case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
            do {
                // Get amount of data transfered
                size = app_usbd_cdc_acm_rx_size(p_cdc_acm);

                // 読み込んだ文字に対して処理を行う
                if (m_rx_buffer[0] == '\n' || m_rx_buffer[0] == '\r') {
                    if (received > 0) {
                        m_line_size = received;
                        m_line_buffer[m_line_size] = 0;
                        NRF_LOG_INFO("Received text [%s](%lu bytes)", m_line_buffer, m_line_size);
                        received = 0;
                    }
                } else {
                    memcpy(m_line_buffer + received, m_rx_buffer, size);
                    received += size;
                }

                // Fetch data until internal buffer is empty
                ret = app_usbd_cdc_acm_read(&m_app_cdc_acm, m_rx_buffer, 1);
            } while (ret == NRF_SUCCESS);
            break;
        default:
            break;
    }
}

static void usbd_user_ev_handler(app_usbd_event_type_t event)
{
    switch (event) {
        case APP_USBD_EVT_DRV_SUSPEND:
            break;
        case APP_USBD_EVT_DRV_RESUME:
            break;
        case APP_USBD_EVT_STARTED:
            break;
        case APP_USBD_EVT_STOPPED:
            app_usbd_disable();
            break;
        case APP_USBD_EVT_POWER_DETECTED:
            NRF_LOG_INFO("USB power detected");
            if (!nrf_drv_usbd_is_enabled()) {
                app_usbd_enable();
            }
            break;
        case APP_USBD_EVT_POWER_REMOVED:
            NRF_LOG_INFO("USB power removed");
            app_usbd_stop();
            break;
        case APP_USBD_EVT_POWER_READY:
            NRF_LOG_INFO("USB ready");
            app_usbd_start();
            break;
        default:
            break;
    }
}

void usbd_init(void)
{
    ret_code_t ret = nrf_drv_clock_init();
    APP_ERROR_CHECK(ret);
    
    nrf_drv_clock_lfclk_request(NULL);
    while (!nrf_drv_clock_lfclk_is_running());

    app_usbd_serial_num_generate();

    static const app_usbd_config_t usbd_config = {
        .ev_state_proc = usbd_user_ev_handler
    };
    ret = app_usbd_init(&usbd_config);
    APP_ERROR_CHECK(ret);

    NRF_LOG_DEBUG("usbd_init() done");
}

void usbd_cdc_init(void)
{
    app_usbd_class_inst_t const * class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&m_app_cdc_acm);
    ret_code_t ret = app_usbd_class_append(class_cdc_acm);
    APP_ERROR_CHECK(ret);

    if (USBD_POWER_DETECTION) {
        ret = app_usbd_power_events_enable();
        APP_ERROR_CHECK(ret);
    } else {
        NRF_LOG_DEBUG("No USB power detection enabled. Starting USB now");
        app_usbd_enable();
        app_usbd_start();
    }

    NRF_LOG_DEBUG("usbd_cdc_init() done");
}

void usbd_cdc_do_process(void)
{
    // USBデバイス処理を実行する
    while (app_usbd_event_queue_process());

    // ログ出力処理を実行
    usbd_cdc_logger_process();
}

ret_code_t usbd_cdc_buffer_write(const void *p_tx_buffer, size_t size)
{
    // 内部バッファにコピーしてから出力
    memcpy(m_tx_buffer, p_tx_buffer, size);
    return app_usbd_cdc_acm_write(&m_app_cdc_acm, p_tx_buffer, size);
}
