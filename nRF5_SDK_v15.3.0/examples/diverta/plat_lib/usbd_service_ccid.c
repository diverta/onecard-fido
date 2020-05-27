/* 
 * File:   usbd_service_ccid.c
 * Author: makmorit
 *
 * Created on 2020/04/27, 11:04
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "app_error.h"

#include "usbd_service.h"
#include "app_usbd_ccid.h"

// for logging informations
#define NRF_LOG_MODULE_NAME usbd_service_ccid
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

//
// CCID関連
//
#include "app_usbd_ccid.h"
//
// Extended APDUフォーマットに対応するための一時バッファ
//   512バイトを上限とします。
//
static uint8_t received_data[512];
static size_t  received_data_size;

// 送信用一時バッファ
static uint8_t send_data[64];

// ATR
// Fi=372, Di=1, 372 cycles/ETU 10752 bits/s at 4.00 MHz
// BWT = 5.7s
static const uint8_t atr_ccid[] = {
    0x3B, 0xF7, 0x11, 0x00, 0x00, 0x81, 0x31, 0xFE, 0x65, 
    0x43, 0x61, 0x6E, 0x6F, 0x6B, 0x65, 0x79, 
    0x99};

//
// APDU処理（仮実装）
//
static void icc_power_on(void)
{
    // レスポンス編集
    memset(send_data, 0, sizeof(send_data));
    // bMessageType
    uint8_t bMessageType = received_data[0];
    send_data[0] = bMessageType;
    // dwLength
    send_data[1] = sizeof(atr_ccid); 
    // abData 
    memcpy(send_data + 10, atr_ccid, sizeof(atr_ccid)); 
    // bSeq
    send_data[6] = received_data[6];

    // レスポンス送信
    if (received_data[6] < 0x0f) {
        app_usbd_ccid_ep_input_from_buffer(send_data, 10 + sizeof(atr_ccid));
    }
}

static void get_slot_status(void)
{
    // レスポンス編集
    memset(send_data, 0, sizeof(send_data));
    // bMessageType
    send_data[0] = 0x61;
    // bSeq
    send_data[6] = received_data[6];

    // レスポンス送信
    if (received_data[6] < 0x0f) {
        app_usbd_ccid_ep_input_from_buffer(send_data, 10);
    }
}

static void apdu_received(void)
{
    uint8_t bMessageType = received_data[0];
    switch (bMessageType) {
        case 0x62:
            // PC_TO_RDR_ICCPOWERON
            icc_power_on();
            break;
        case 0x65:
            // PC_TO_RDR_GETSLOTSTATUS
            get_slot_status();
            break;
        default:
            break;
    }

}

static void ccid_user_ev_handler(app_usbd_class_inst_t const *p_inst, enum app_usbd_ccid_user_event_e event)
{
    switch (event) {
        case APP_USBD_CCID_USER_EVT_RX_DONE:
            // フレームデータをバッファに退避
            memcpy(received_data + received_data_size, 
                app_usbd_ccid_ep_output_buffer(), app_usbd_ccid_ep_output_buffer_size());
            received_data_size += app_usbd_ccid_ep_output_buffer_size();

            // APDUの処理を実行
            apdu_received();
            received_data_size = 0;
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
    
    received_data_size = 0;
    NRF_LOG_DEBUG("usbd_ccid_init() done");
}
