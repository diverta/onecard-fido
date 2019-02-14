/* 
 * File:   fido_ble_central_nus.c
 * Author: makmorit
 *
 * Created on 2019/02/14, 11:19
 */
//
// TODO:
// このプログラムは、
// USB HIDとBLEセントラル同居に関する動作確認用であり、
// 正式な要求仕様にもとづいた実装ではありません。
// ですので、FIDO2対応のmasterブランチに登録する際は、
// 機能を無効化するようお願いいたします。
//
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "nordic_common.h"
#include "app_error.h"
#include "ble_db_discovery.h"
#include "ble.h"
#include "ble_gap.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_scan.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_ble_central_nus
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for BLE NUS service client
#include "ble_nus_c.h"

/**< UUID type for the Nordic UART Service (vendor specific). */
#define NUS_SERVICE_UUID_TYPE   BLE_UUID_TYPE_VENDOR_BEGIN

/**< BLE Nordic UART Service (NUS) client instance. */
BLE_NUS_C_DEF(m_ble_nus_c);

/**@brief NUS UUID. */
static ble_uuid_t const m_nus_uuid = {
    .uuid = BLE_UUID_NUS_SERVICE,
    .type = NUS_SERVICE_UUID_TYPE
};

ble_uuid_t const *fido_ble_central_nus_uuid(void)
{
    return &m_nus_uuid;
}

static void ble_nus_c_evt_handler(ble_nus_c_t *p_ble_nus_c, ble_nus_c_evt_t const *p_ble_nus_evt)
{
    ret_code_t err_code;

    switch (p_ble_nus_evt->evt_type) {
        case BLE_NUS_C_EVT_DISCOVERY_COMPLETE:
            NRF_LOG_INFO("Discovery complete.");
            err_code = ble_nus_c_handles_assign(p_ble_nus_c, p_ble_nus_evt->conn_handle, &p_ble_nus_evt->handles);
            APP_ERROR_CHECK(err_code);

            err_code = ble_nus_c_tx_notif_enable(p_ble_nus_c);
            APP_ERROR_CHECK(err_code);
            NRF_LOG_INFO("Connected to device with Nordic UART Service.");
            break;

        case BLE_NUS_C_EVT_NUS_TX_EVT:
            // TODO:
            // 受信したデータをプリントするなどの処理を実行
            // ble_nus_chars_received_uart_print(p_ble_nus_evt->p_data, p_ble_nus_evt->data_len);
            break;

        case BLE_NUS_C_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected.");
            break;
    }
}

void fido_ble_central_nus_init(void)
{
    ret_code_t       err_code;
    ble_nus_c_init_t init;

    init.evt_handler = ble_nus_c_evt_handler;

    err_code = ble_nus_c_init(&m_ble_nus_c, &init);
    APP_ERROR_CHECK(err_code);
}

void fido_ble_central_nus_on_db_disc_evt(ble_db_discovery_evt_t *p_evt)
{
    ble_nus_c_on_db_disc_evt(&m_ble_nus_c, p_evt);
}

void fido_ble_central_nus_evt_connected(ble_evt_t *p_ble_evt, void *p_context)
{
    ret_code_t err_code;

    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            err_code = ble_nus_c_handles_assign(&m_ble_nus_c, p_ble_evt->evt.gap_evt.conn_handle, NULL);
            APP_ERROR_CHECK(err_code);
    }
}
