/* 
 * File:   nfc_service.c
 * Author: makmorit
 *
 * Created on 2019/05/28, 14:21
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "sdk_common.h"
#include "nfc_t4t_lib.h"
#include "app_error.h"

#include "nfc_common.h"
#include "nfc_fido_receive.h"

// for logging informations
#define NRF_LOG_MODULE_NAME nfc_service
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for debug hid report
#define NRF_LOG_HEXDUMP_DEBUG_RX_APDU false
#define NRF_LOG_HEXDUMP_DEBUG_TX_APDU false
#define NRF_LOG_DEBUG_BUFF (NRF_LOG_HEXDUMP_DEBUG_RX_APDU || NRF_LOG_HEXDUMP_DEBUG_TX_APDU)

// NFCの接続状態を保持
static bool nfc_field_on;

// Extended APDUフォーマットに対応するための一時バッファ
static uint8_t received_data[NFC_APDU_BUFF_SIZE];
static size_t  received_data_size;

#if NRF_LOG_DEBUG_BUFF
static void print_hexdump_debug(uint8_t *buff, size_t size)
{
    int j, k;
    for (j = 0; j < size; j += 64) {
        k = size - j;
        NRF_LOG_HEXDUMP_DEBUG(buff + j, (k < 64) ? k : 64);
    }
}
#endif

static void nfc_data_received(const uint8_t *data, size_t data_size)
{
#if NRF_LOG_HEXDUMP_DEBUG_RX_APDU
    NRF_LOG_DEBUG("NFC RX data (%d bytes):", data_size);
    print_hexdump_debug(data, data_size);
#endif

    nfc_fido_receive_request_frame((uint8_t *)data, data_size);
}

static void nfc_callback(void *context, nfc_t4t_event_t event, const uint8_t *data, size_t data_size, uint32_t flags)
{
    UNUSED_PARAMETER(context);

    switch (event) {
        case NFC_T4T_EVENT_FIELD_ON:
            // NFCセッションが開始された時の処理
            if (nfc_field_on == false) {
                NRF_LOG_INFO("NFC Tag has been selected");
                nfc_field_on = true;
                received_data_size = 0;
            }
            break;

        case NFC_T4T_EVENT_FIELD_OFF:
            // NFCセッションが切断された時の処理
            if (nfc_field_on == true) {
                NRF_LOG_INFO("NFC field lost");
                nfc_field_on = false;
            }
            break;

        case NFC_T4T_EVENT_DATA_IND:
            // フレームデータをバッファに退避
            memcpy(received_data + received_data_size, data, data_size);
            received_data_size += data_size;

            // 全フレーム受信完了時の処理
            if (flags != NFC_T4T_DI_FLAG_MORE) {
                nfc_data_received(received_data, received_data_size);
                received_data_size = 0;
            }
            break;

        default:
            break;
    }
}

void nfc_service_init(void)
{
    nfc_field_on = false;

    // Set up NFC
    ret_code_t err_code = nfc_t4t_setup(nfc_callback, NULL);
    APP_ERROR_CHECK(err_code);

    // Start sensing NFC field
    err_code = nfc_t4t_emulation_start();
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("Set up NFC and started sensing NFC field");
}

void nfc_service_data_send(uint8_t *data, size_t data_size)
{
#if NRF_LOG_HEXDUMP_DEBUG_TX_APDU
    NRF_LOG_DEBUG("NFC TX data (%d bytes):", data_size);
    print_hexdump_debug(data, data_size);
#endif

    // Send the response PDU over NFC.
    ret_code_t err_code = nfc_t4t_response_pdu_send(data, data_size);
    APP_ERROR_CHECK(err_code);
}
