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
#include "nrf_ble_scan.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ble_service_central_stat
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for debug log and hexdump
#define LOG_HEXDUMP_DEBUG_ADVDATA   false

// 最大 3 デバイス分の
// アドバタイジング情報を保持
#define ADV_STAT_INFO_SIZE_MAX 3
#define ADV_STAT_INFO_DATA_MAX_SIZE 32
#define UUID_BYTES_MAX_SIZE 16
typedef struct {
    uint8_t peer_addr[BLE_GAP_ADDR_LEN];
    int8_t  rssi;
    int8_t  tx_power;
    uint8_t ad_type;
    uint8_t dev_name[ADV_STAT_INFO_DATA_MAX_SIZE];
    size_t  dev_name_size;
    uint8_t uuid_bytes[UUID_BYTES_MAX_SIZE];
    size_t  uuid_bytes_size;
} ADV_STAT_INFO_T;
static ADV_STAT_INFO_T adv_stat_info[ADV_STAT_INFO_SIZE_MAX];
static uint8_t         adv_stat_info_size = 0;

// 比較用のゼロアドレス
static uint8_t zero_addr[] = {0, 0, 0, 0, 0, 0};

void ble_service_central_stat_info_init(void)
{
    memset(&adv_stat_info, 0, sizeof(ADV_STAT_INFO_T) * ADV_STAT_INFO_SIZE_MAX);
    adv_stat_info_size = 0;
}

static void debug_print_adv_stat_info(ADV_STAT_INFO_T *info)
{
    NRF_LOG_DEBUG("Bluetooth address:%02x%02x%02x%02x%02x%02x",
        info->peer_addr[5],
        info->peer_addr[4],
        info->peer_addr[3],
        info->peer_addr[2],
        info->peer_addr[1],
        info->peer_addr[0]
        );
    NRF_LOG_DEBUG("Device name:%s (TX power=%d, RSSI=%d), UUID(%d bytes):", 
        info->dev_name, info->tx_power, info->rssi, info->uuid_bytes_size);
    NRF_LOG_HEXDUMP_DEBUG(info->uuid_bytes, info->uuid_bytes_size);
}

void ble_service_central_stat_debug_print(void)
{
    for (uint8_t idx = 0; idx < adv_stat_info_size; idx++) {
        debug_print_adv_stat_info(&adv_stat_info[idx]);
    }
}

static void set_adv_stat_info(ADV_STAT_INFO_T *info, ble_gap_evt_adv_report_t const *p_adv_report)
{
    // Bluetoothアドレス、RSSI、Tx Power Levelを設定
    memcpy(info->peer_addr, p_adv_report->peer_addr.addr, BLE_GAP_ADDR_LEN);
    info->rssi = p_adv_report->rssi;
    info->tx_power = p_adv_report->tx_power;

#if LOG_HEXDUMP_DEBUG_ADVDATA
    NRF_LOG_DEBUG("Bluetooth address %02x%02x%02x%02x%02x%02x",
        p_adv_report->peer_addr.addr[5],
        p_adv_report->peer_addr.addr[4],
        p_adv_report->peer_addr.addr[3],
        p_adv_report->peer_addr.addr[2],
        p_adv_report->peer_addr.addr[1],
        p_adv_report->peer_addr.addr[0]
        );
    NRF_LOG_DEBUG("Advertise data (%d bytes)=", p_adv_report->data.len);
    NRF_LOG_HEXDUMP_DEBUG(p_adv_report->data.p_data, p_adv_report->data.len);
#endif

    size_t index = 0;
    uint8_t * p_data = p_adv_report->data.p_data;
    size_t data_size = p_adv_report->data.len;
    
    while (index < data_size) {
        uint8_t field_length = p_data[index];
        uint8_t field_type = p_data[index + 1];
        uint8_t *p_field_data = p_data + index + 2;
        uint8_t field_data_size = field_length - 1;
        
        info->ad_type = field_type;
        switch (field_type) {
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                // UUIDバイト配列
                // ビッグエンディアン形式で格納させる
                info->uuid_bytes_size = field_data_size;
                for (uint8_t c = 0; c < field_data_size; c++) {
                    info->uuid_bytes[field_data_size - 1 - c] = p_field_data[c];
                }
                break;
            case 8:
            case 9:
                // デバイス名称
                info->dev_name_size = field_data_size;
                memcpy(info->dev_name, p_field_data, field_data_size);
                info->dev_name[field_data_size] = 0;
                break;
            default:
                break;
        }
        index += field_length + 1;
    }
}

void ble_service_central_stat_adv_report(ble_gap_evt_adv_report_t const *p_adv_report)
{
    for (uint8_t idx = 0; idx < ADV_STAT_INFO_SIZE_MAX; idx++) {
        // 同一のアドレスが出現したら、その位置に上書き
        if (memcmp(adv_stat_info[idx].peer_addr, p_adv_report->peer_addr.addr, BLE_GAP_ADDR_LEN) == 0) {
            set_adv_stat_info(&adv_stat_info[idx], p_adv_report);
            return;
        }
        // ブランクが出現したら、その位置に新規追加し、データ数を設定
        if (memcmp(adv_stat_info[idx].peer_addr, zero_addr, BLE_GAP_ADDR_LEN) == 0) {
            set_adv_stat_info(&adv_stat_info[idx], p_adv_report);
            adv_stat_info_size = idx + 1;
            return;
        }
    }
}
