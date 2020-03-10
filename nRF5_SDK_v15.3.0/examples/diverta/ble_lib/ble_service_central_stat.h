/* 
 * File:   ble_service_central_stat.h
 * Author: makmorit
 *
 * Created on 2019/10/03, 16:20
 */
#ifndef BLE_SERVICE_CENTRAL_STAT_H
#define BLE_SERVICE_CENTRAL_STAT_H

#include <stdbool.h>
#include "ble_gap.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// アドバタイジング情報から取得した統計情報
//
#define ADV_STAT_INFO_DATA_MAX_SIZE 32
#define ADV_STAT_INFO_UUID_MAX_SIZE 16
typedef struct {
    uint8_t peer_addr[BLE_GAP_ADDR_LEN];
    int8_t  rssi;
    int8_t  tx_power;
    uint8_t ad_type;
    uint8_t dev_name[ADV_STAT_INFO_DATA_MAX_SIZE];
    size_t  dev_name_size;
    uint8_t uuid_bytes[ADV_STAT_INFO_UUID_MAX_SIZE];
    size_t  uuid_bytes_size;
} ADV_STAT_INFO_T;

void ble_service_central_stat_info_init(void);
void ble_service_central_stat_adv_report(ble_gap_evt_adv_report_t const *p_adv_report);
void ble_service_central_stat_debug_print(void);

ADV_STAT_INFO_T *ble_service_central_stat_match_uuid(char *uuid_strict_string);
ADV_STAT_INFO_T *ble_service_central_stat_match_scan_param(uint8_t *p_scan_param);
char            *ble_service_central_stat_btaddr_string(uint8_t *addr_bytes);
size_t           ble_service_central_stat_csv_get(uint32_t serial_num, char *adv_stat_info_string);

#ifdef __cplusplus
}
#endif

#endif /* BLE_SERVICE_CENTRAL_STAT_H */
