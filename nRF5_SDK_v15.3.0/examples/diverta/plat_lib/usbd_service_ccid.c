/* 
 * File:   usbd_service_ccid.c
 * Author: makmorit
 *
 * Created on 2020/04/27, 11:04
 */
#include "app_error.h"

//
// CCID関連
//
#include "usbd_service.h"
#include "app_usbd_ccid.h"
#include "ccid.h"

// for logging informations
#define NRF_LOG_MODULE_NAME usbd_service_ccid
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for debug buffer
#define NRF_LOG_HEXDUMP_DEBUG_BUFFER false

static void ccid_user_ev_handler(app_usbd_class_inst_t const *p_inst, enum app_usbd_ccid_user_event_e event)
{
    switch (event) {
        case APP_USBD_CCID_USER_EVT_RX_DONE:
#if NRF_LOG_HEXDUMP_DEBUG_BUFFER
            NRF_LOG_DEBUG("usbd_ccid_data_frame_received(%d bytes):", app_usbd_ccid_ep_output_buffer_size());
            NRF_LOG_HEXDUMP_DEBUG(app_usbd_ccid_ep_output_buffer(), app_usbd_ccid_ep_output_buffer_size());
#endif
            // 受信フレームデータを処理
            ccid_data_frame_received(
                app_usbd_ccid_ep_output_buffer(), 
                app_usbd_ccid_ep_output_buffer_size());
            break;
        default:
            break;
    }
}

APP_USBD_CCID_GLOBAL_DEF(m_app_inst_ccid, ccid_user_ev_handler, CCID_DATA_INTERFACE, CCID_DATA_EPIN, CCID_DATA_EPOUT);

void usbd_ccid_init(void)
{
    app_usbd_class_inst_t const * class_ccid = app_usbd_ccid_class_inst_get(&m_app_inst_ccid);
    ret_code_t ret = app_usbd_class_append(class_ccid);
    if (ret != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_usbd_class_append(class_ccid) returns 0x%02x ", ret);
    }
    APP_ERROR_CHECK(ret);

    // モジュール変数を初期化
    ccid_initialize_value();
    NRF_LOG_DEBUG("usbd_ccid_init() done");
}

void usbd_ccid_send_data_frame(uint8_t *p_data, size_t size)
{
    app_usbd_ccid_ep_input_from_buffer(p_data, size);

#if NRF_LOG_HEXDUMP_DEBUG_BUFFER
    NRF_LOG_DEBUG("usbd_ccid_send_data_frame(%d bytes)", size);
    NRF_LOG_HEXDUMP_DEBUG(p_data, size);
#endif
}
