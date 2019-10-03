/* 
 * File:   ble_service_central_stat.h
 * Author: makmorit
 *
 * Created on 2019/10/03, 16:20
 */
#ifndef BLE_SERVICE_CENTRAL_STAT_H
#define BLE_SERVICE_CENTRAL_STAT_H

#ifdef __cplusplus
extern "C" {
#endif

void ble_service_central_stat_info_init(void);
void ble_service_central_stat_adv_report(ble_gap_evt_adv_report_t const *p_adv_report);

void ble_service_central_stat_debug_print(void);

#ifdef __cplusplus
}
#endif

#endif /* BLE_SERVICE_CENTRAL_STAT_H */
