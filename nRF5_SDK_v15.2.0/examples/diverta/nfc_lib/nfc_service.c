/* 
 * File:   nfc_service.c
 * Author: makmorit
 *
 * Created on 2019/05/28, 14:21
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "nfc_t4t_lib.h"
#include "app_error.h"

#include "nfc_service.h"

// for logging informations
#define NRF_LOG_MODULE_NAME nfc_service
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// NFCの接続状態を保持
static bool nfc_field_on;

static void nfc_callback(void *context, nfc_t4t_event_t event, const uint8_t *data, size_t data_size, uint32_t flags)
{
    ret_code_t      err_code;
    uint32_t        resp_len;
    static uint8_t  apdu_buf[NFC_APDU_BUFF_SIZE];

    UNUSED_PARAMETER(context);

    switch (event) {
        case NFC_T4T_EVENT_FIELD_ON:
            // NFCセッションが開始された時の処理
            if (nfc_field_on == false) {
                NRF_LOG_INFO("NFC Tag has been selected");
                nfc_field_on = true;
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
            if (flags == NFC_T4T_DI_FLAG_MORE) {
                // 通信中の場合は何もしない
                break;
            }

            NRF_LOG_DEBUG("NFC RX data (%d bytes):", data_size);
            NRF_LOG_HEXDUMP_DEBUG(data, data_size);

            // OK: 0x9000
            apdu_buf[0] = 0x90;
            apdu_buf[1] = 0x00;
            resp_len = 2;

            // for debug
            NRF_LOG_DEBUG("NFC TX data (%d bytes):", resp_len);
            NRF_LOG_HEXDUMP_DEBUG(apdu_buf, resp_len);

            // Send the response PDU over NFC.
            err_code = nfc_t4t_response_pdu_send(apdu_buf, resp_len);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            break;
    }
}

void nfc_func_init(void)
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
