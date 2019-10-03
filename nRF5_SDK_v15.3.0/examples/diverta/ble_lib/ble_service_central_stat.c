/* 
 * File:   ble_service_central_stat.c
 * Author: makmorit
 *
 * Created on 2019/10/03, 16:20
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
#define NRF_LOG_MODULE_NAME ble_service_central_stat
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// 最大 3 デバイス分の
// アドバタイジング情報を保持
#define ADV_STAT_INFO_SIZE_MAX 3
#define ADV_STAT_INFO_DATA_MAX_SIZE 32
typedef struct {
    uint8_t peer_addr[BLE_GAP_ADDR_LEN];
    int8_t  rssi;
    uint8_t data_buff[ADV_STAT_INFO_DATA_MAX_SIZE];
    size_t  data_size;
} ADV_STAT_INFO_T;
static ADV_STAT_INFO_T adv_stat_info[ADV_STAT_INFO_SIZE_MAX];
static uint8_t         adv_stat_info_size = 0;

// 比較用のゼロアドレス
static uint8_t zero_addr[] = {0, 0, 0, 0, 0, 0};

void ble_service_central_stat_info_init(void)
{
    memset(&adv_stat_info, 0, sizeof(ADV_STAT_INFO_T) * ADV_STAT_INFO_SIZE_MAX);
}

void ble_service_central_stat_debug_print(void)
{
    for (uint8_t idx = 0; idx < adv_stat_info_size; idx++) {
        NRF_LOG_DEBUG("BT ADDR %02x%02x%02x%02x%02x%02x",
            adv_stat_info[idx].peer_addr[5],
            adv_stat_info[idx].peer_addr[4],
            adv_stat_info[idx].peer_addr[3],
            adv_stat_info[idx].peer_addr[2],
            adv_stat_info[idx].peer_addr[1],
            adv_stat_info[idx].peer_addr[0]
            );
        NRF_LOG_DEBUG("rssi:%d , adv data (%d bytes):", adv_stat_info[idx].rssi, adv_stat_info[idx].data_size);
        NRF_LOG_HEXDUMP_DEBUG(adv_stat_info[idx].data_buff, adv_stat_info[idx].data_size);
    }
}

static void set_adv_stat_info(uint8_t idx, ble_gap_evt_adv_report_t const *p_adv_report)
{
    memcpy(adv_stat_info[idx].peer_addr, p_adv_report->peer_addr.addr, BLE_GAP_ADDR_LEN);
    adv_stat_info[idx].rssi = p_adv_report->rssi;
    
    adv_stat_info[idx].data_size = p_adv_report->data.len > ADV_STAT_INFO_DATA_MAX_SIZE ? 
        ADV_STAT_INFO_DATA_MAX_SIZE : p_adv_report->data.len;
    memcpy(adv_stat_info[idx].data_buff, p_adv_report->data.p_data, adv_stat_info[idx].data_size);

        NRF_LOG_DEBUG("BT ADDR %02x%02x%02x%02x%02x%02x",
            p_adv_report->peer_addr.addr[5],
            p_adv_report->peer_addr.addr[4],
            p_adv_report->peer_addr.addr[3],
            p_adv_report->peer_addr.addr[2],
            p_adv_report->peer_addr.addr[1],
            p_adv_report->peer_addr.addr[0]
            );
        
    NRF_LOG_DEBUG("ch_index=0x%02x, data_id=0x%04x, set_id=0x%02x, data (%d bytes)=", 
        p_adv_report->ch_index,
        p_adv_report->data_id, p_adv_report->set_id, p_adv_report->data.len);
    NRF_LOG_HEXDUMP_DEBUG(p_adv_report->data.p_data, p_adv_report->data.len);
}

void ble_service_central_stat_adv_report(ble_gap_evt_adv_report_t const *p_adv_report)
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
}
