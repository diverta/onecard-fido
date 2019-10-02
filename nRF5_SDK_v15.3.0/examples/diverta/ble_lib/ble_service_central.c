/* 
 * File:   ble_service_central.c
 * Author: makmorit
 *
 * Created on 2019/10/02, 14:47
 */
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_gap.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_scan.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ble_service_central
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define APP_BLE_CONN_CFG_TAG 1
NRF_BLE_SCAN_DEF(m_scan);

static void scan_evt_handler(scan_evt_t const *p_scan_evt)
{
    ret_code_t err_code;

    switch(p_scan_evt->scan_evt_id) {
        case NRF_BLE_SCAN_EVT_FILTER_MATCH:
            NRF_LOG_DEBUG("Scan filter matched");
            break;

        case NRF_BLE_SCAN_EVT_CONNECTING_ERROR:
            err_code = p_scan_evt->params.connecting_err.err_code;
            NRF_LOG_ERROR("Scan connecting error: code=0x%02x", err_code);
            break;

        case NRF_BLE_SCAN_EVT_CONNECTED:
            // Scan is automatically stopped by the connection.
            break;

        case NRF_BLE_SCAN_EVT_SCAN_TIMEOUT:
            NRF_LOG_ERROR("Scan timed out.");
            break;

         default:
             break;
    }
}

static void scan_init(void)
{
    ret_code_t          err_code;
    nrf_ble_scan_init_t init_scan;

    memset(&init_scan, 0, sizeof(init_scan));

    init_scan.connect_if_match = false;
    init_scan.conn_cfg_tag     = APP_BLE_CONN_CFG_TAG;

    err_code = nrf_ble_scan_init(&m_scan, &init_scan, scan_evt_handler);
    APP_ERROR_CHECK(err_code);
}

void ble_service_central_init(void)
{
    scan_init();
    NRF_LOG_DEBUG("BLE central initialized");
}

void ble_service_central_scan_filter_set(void)
{
    ble_uuid_t const uuid = {
        .uuid = 0x0001,
        .type = BLE_UUID_TYPE_VENDOR_BEGIN
    };

    ret_code_t err_code = nrf_ble_scan_filter_set(&m_scan, SCAN_UUID_FILTER, &uuid);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_scan_filters_enable(&m_scan, NRF_BLE_SCAN_UUID_FILTER, false);
    APP_ERROR_CHECK(err_code);
}

void ble_service_central_scan_start(void)
{
    ret_code_t ret = nrf_ble_scan_start(&m_scan);
    APP_ERROR_CHECK(ret);

    // スキャン中の旨を通知
    NRF_LOG_DEBUG("Scan started");
}

void ble_service_central_scan_stop(void)
{
    // Stop scanning.
    nrf_ble_scan_stop();
    NRF_LOG_DEBUG("Scan stopped");
}

//
// BLE GAPイベント関連処理
//
void ble_service_central_gap_adv_report(ble_evt_t const *p_ble_evt)
{
    // TODO:
    // 取得したRSSI値をログ出力対象に設定
}
