/* 
 * File:   fido_ble_central.c
 * Author: makmorit
 *
 * Created on 2019/02/11, 16:59
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "nordic_common.h"
#include "app_error.h"
#include "ble_db_discovery.h"
#include "ble.h"
#include "ble_gap.h"
#include "ble_srv_common.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_scan.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_ble_central
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for fido_ble_peripheral_mode
#include "fido_ble_peripheral.h"

// BLEパケット項目のサイズ
#define OPCODE_LENGTH 1
#define HANDLE_LENGTH 2

// Tag that refers to the BLE stack configuration set with 
// @ref sd_ble_cfg_set. The default tag is @ref BLE_CONN_CFG_TAG_DEFAULT.
#define APP_BLE_CONN_CFG_TAG 1   

static ble_uuid_t const m_uuid = {
    .uuid = BLE_UUID_CURRENT_TIME_SERVICE,
    .type = BLE_UUID_TYPE_BLE
};

BLE_DB_DISCOVERY_DEF(m_db_disc);    /**< Database discovery module instance. */
NRF_BLE_SCAN_DEF(m_scan);           /**< Scanning Module instance. */

// スキャン結果を保持するフラグ
static bool scan_not_found_flag;

void fido_ble_central_scan_start(void)
{
    if (fido_ble_peripheral_mode()) {
        return;
    }

    ret_code_t ret;
    scan_not_found_flag = false;

    ret = nrf_ble_scan_start(&m_scan);
    APP_ERROR_CHECK(ret);

    // スキャン中の旨を通知
    NRF_LOG_DEBUG("Scan started");
}

void fido_ble_central_scan_stop(void)
{
    if (fido_ble_peripheral_mode()) {
        return;
    }

    // Stop scanning.
    nrf_ble_scan_stop();
    NRF_LOG_DEBUG("Scan stopped");
}

static void db_disc_handler(ble_db_discovery_evt_t *p_evt)
{
}

static void db_discovery_init(void)
{
    ret_code_t err_code = ble_db_discovery_init(db_disc_handler);
    APP_ERROR_CHECK(err_code);
}

static void scan_evt_handler(scan_evt_t const *p_scan_evt)
{
    ret_code_t err_code;
    ble_gap_evt_connected_t const *p_connected;

    switch(p_scan_evt->scan_evt_id) {
        case NRF_BLE_SCAN_EVT_CONNECTING_ERROR:
            err_code = p_scan_evt->params.connecting_err.err_code;
            NRF_LOG_ERROR("Scan connecting error: code=0x%02x", err_code);
            break;

        case NRF_BLE_SCAN_EVT_CONNECTED:
            p_connected = p_scan_evt->params.connected.p_connected;
            // Scan is automatically stopped by the connection.
            NRF_LOG_DEBUG("Connecting to target %02x%02x%02x%02x%02x%02x",
                p_connected->peer_addr.addr[0],
                p_connected->peer_addr.addr[1],
                p_connected->peer_addr.addr[2],
                p_connected->peer_addr.addr[3],
                p_connected->peer_addr.addr[4],
                p_connected->peer_addr.addr[5]
                );
            break;

        case NRF_BLE_SCAN_EVT_SCAN_TIMEOUT:
            NRF_LOG_ERROR("Scan timed out.");
            break;

        case NRF_BLE_SCAN_EVT_FILTER_MATCH:
            NRF_LOG_DEBUG("Scan filter matched");
            scan_not_found_flag = false;
            break;
            
        case NRF_BLE_SCAN_EVT_NOT_FOUND:
            if (scan_not_found_flag == false) {
                scan_not_found_flag = true;
                NRF_LOG_DEBUG("Scan not found");
            }
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

    init_scan.connect_if_match = true;
    init_scan.conn_cfg_tag     = APP_BLE_CONN_CFG_TAG;

    err_code = nrf_ble_scan_init(&m_scan, &init_scan, scan_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_scan_filter_set(&m_scan, SCAN_UUID_FILTER, &m_uuid);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_scan_filters_enable(&m_scan, NRF_BLE_SCAN_UUID_FILTER, false);
    APP_ERROR_CHECK(err_code);
}

void fido_ble_central_init(void)
{
    if (fido_ble_peripheral_mode()) {
        return;
    }

    db_discovery_init();
    scan_init();
    NRF_LOG_DEBUG("BLE central initialized");
}

void fido_ble_central_evt_handler(ble_evt_t *p_ble_evt, void *p_context)
{
    UNUSED_PARAMETER(p_context);
    if (fido_ble_peripheral_mode()) {
        return;
    }

    ret_code_t           err_code;
    ble_gap_evt_t const *p_gap_evt = &p_ble_evt->evt.gap_evt;

    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            // 接続中の旨を通知
            NRF_LOG_DEBUG("Connected");

            // start discovery of services. The NUS Client waits for a discovery result
            err_code = ble_db_discovery_start(&m_db_disc, p_ble_evt->evt.gap_evt.conn_handle);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_DEBUG("Disconnected. conn_handle: 0x%x, reason: 0x%x",
                         p_gap_evt->conn_handle,
                         p_gap_evt->params.disconnected.reason);
            break;

        case BLE_GAP_EVT_TIMEOUT:
            if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN) {
                NRF_LOG_DEBUG("Connection Request timed out.");
            }
            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported.
            err_code = sd_ble_gap_sec_params_reply(p_ble_evt->evt.gap_evt.conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
            // Accepting parameters requested by peer.
            err_code = sd_ble_gap_conn_param_update(p_gap_evt->conn_handle,
                                                    &p_gap_evt->params.conn_param_update_request.conn_params);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys = {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            NRF_LOG_DEBUG("GATT Client Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            NRF_LOG_DEBUG("GATT Server Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            break;
    }
}

void fido_ble_central_gatt_evt_handler(nrf_ble_gatt_t *p_gatt, nrf_ble_gatt_evt_t const *p_evt)
{
    UNUSED_PARAMETER(p_gatt);
    if (fido_ble_peripheral_mode()) {
        return;
    }

    if (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED) {
        uint16_t max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_DEBUG("ATT MTU exchange completed: Max data length set to 0x%X(%d)", max_data_len, max_data_len);
    }
}
