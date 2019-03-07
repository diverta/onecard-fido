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
#include "ble.h"
#include "ble_gap.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_scan.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_ble_central
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for usbd_cdc_logger_process_set_rssi
#include "usbd_cdc_logger_process.h"

// BLEパケット項目のサイズ
#define OPCODE_LENGTH 1
#define HANDLE_LENGTH 2

#define APP_BLE_CONN_CFG_TAG    1   /**< Tag that refers to the BLE stack configuration set with @ref sd_ble_cfg_set. The default tag is @ref BLE_CONN_CFG_TAG_DEFAULT. */

NRF_BLE_SCAN_DEF(m_scan);           /**< Scanning Module instance. */

// 最大 3 デバイス分の
// アドバタイジング情報を保持
#define ADV_STAT_INFO_SIZE_MAX 3
typedef struct {
    uint8_t peer_addr[BLE_GAP_ADDR_LEN];
    int8_t  rssi;
} ADV_STAT_INFO_T;
static ADV_STAT_INFO_T adv_stat_info[ADV_STAT_INFO_SIZE_MAX];
static uint8_t         adv_stat_info_size = 0;

// 比較用のゼロアドレス
static uint8_t zero_addr[] = {0, 0, 0, 0, 0, 0};

// Bluetoothアドレス、RSSI値の編集用領域
static char peer_addr_rssi_buf[32];

// for BLE NUS service client
#include "ble_nus_c.h"

/**@brief NUS UUID. */
static ble_uuid_t const m_nus_uuid = {
    .uuid = BLE_UUID_NUS_SERVICE,
    .type = BLE_UUID_TYPE_VENDOR_BEGIN
};

static ble_uuid_t const *get_ble_nus_uuid(void)
{
    return &m_nus_uuid;
}

void fido_ble_central_scan_start(void)
{
    ret_code_t ret;

    ret = nrf_ble_scan_start(&m_scan);
    APP_ERROR_CHECK(ret);

    // スキャン中の旨を通知
    NRF_LOG_DEBUG("Scan started");
}

void fido_ble_central_scan_stop(void)
{
    // Stop scanning.
    nrf_ble_scan_stop();
    NRF_LOG_DEBUG("Scan stopped");
}

static void scan_evt_handler(scan_evt_t const *p_scan_evt)
{
    ret_code_t err_code;

    switch(p_scan_evt->scan_evt_id) {
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

    err_code = nrf_ble_scan_filter_set(&m_scan, SCAN_UUID_FILTER, get_ble_nus_uuid());
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_scan_filters_enable(&m_scan, NRF_BLE_SCAN_UUID_FILTER, false);
    APP_ERROR_CHECK(err_code);
}

void fido_ble_central_init(void)
{
    memset(&adv_stat_info, 0, sizeof(ADV_STAT_INFO_T) * ADV_STAT_INFO_SIZE_MAX);
    scan_init();
    NRF_LOG_DEBUG("BLE central initialized");
}

static void set_adv_stat_info(uint8_t idx, ble_gap_evt_adv_report_t const *p_adv_report)
{
    memcpy(adv_stat_info[idx].peer_addr, p_adv_report->peer_addr.addr, BLE_GAP_ADDR_LEN);
    adv_stat_info[idx].rssi = p_adv_report->rssi;
    
}

static void clear_adv_stat_info(uint8_t idx)
{
    memcpy(adv_stat_info[idx].peer_addr, zero_addr, BLE_GAP_ADDR_LEN);
    adv_stat_info[idx].rssi = 0;
}

static void update_adv_stat_info(ble_gap_evt_adv_report_t const *p_adv_report)
{
    for (uint8_t idx = 0; idx < ADV_STAT_INFO_SIZE_MAX; idx++) {
        // 同一のアドレスが出現したら、その位置に上書き
        if (memcmp(adv_stat_info[idx].peer_addr, p_adv_report->peer_addr.addr, BLE_GAP_ADDR_LEN) == 0) {
            set_adv_stat_info(idx, p_adv_report);
            return;
        }
        // ブランクが出現したら、その位置に新規追加し、データ数を設定
        if (memcmp(adv_stat_info[idx].peer_addr, zero_addr, BLE_GAP_ADDR_LEN) == 0) {
            set_adv_stat_info(idx, p_adv_report);
            adv_stat_info_size = idx + 1;
            return;
        }
    }
    // データ追加が行われなかった旨をログ出力
    NRF_LOG_DEBUG("Advertising statistics info not set");
}

void fido_ble_central_evt_handler(ble_evt_t *p_ble_evt, void *p_context)
{
    UNUSED_PARAMETER(p_context);
    ble_gap_evt_t const *p_gap_evt = &p_ble_evt->evt.gap_evt;

    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_ADV_REPORT:
            // 取得したRSSI値をログ出力対象に設定
            update_adv_stat_info(&p_gap_evt->params.adv_report);
            break;
        default:
            break;
    }
}

void fido_ble_central_gatt_evt_handler(nrf_ble_gatt_t *p_gatt, nrf_ble_gatt_evt_t const *p_evt)
{
    UNUSED_PARAMETER(p_gatt);

    if (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED) {
        uint16_t max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_DEBUG("ATT MTU exchange completed: Max data length set to 0x%X(%d)", max_data_len, max_data_len);
    }
}

size_t get_adv_stat_info_string(uint32_t serial_num, char *adv_stat_info_string)
{
    size_t size;

    // 通番を編集
    sprintf(adv_stat_info_string, "%lu,", serial_num);

    for (uint8_t idx = 0; idx < ADV_STAT_INFO_SIZE_MAX; idx++) {
        // Bluetoothアドレス、RSSI値を編集
        sprintf(peer_addr_rssi_buf, 
            "%02x%02x%02x%02x%02x%02x,%d",
            adv_stat_info[idx].peer_addr[5],
            adv_stat_info[idx].peer_addr[4],
            adv_stat_info[idx].peer_addr[3],
            adv_stat_info[idx].peer_addr[2],
            adv_stat_info[idx].peer_addr[1],
            adv_stat_info[idx].peer_addr[0],
            adv_stat_info[idx].rssi
            );

        // CSVデータを編集
        size = sprintf(adv_stat_info_string, 
            "%s%s%s", 
            adv_stat_info_string, 
            memcmp(adv_stat_info[idx].peer_addr, zero_addr, BLE_GAP_ADDR_LEN) ? peer_addr_rssi_buf : ",",
            (idx == ADV_STAT_INFO_SIZE_MAX - 1) ? "\r\n" : ","
            );
        clear_adv_stat_info(idx);
    }

    return size;
}