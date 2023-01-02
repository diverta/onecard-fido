/* 
 * File:   fido_ble_service_define.h
 * Author: makmorit
 *
 * Created on 2023/01/02, 11:45
 */
#ifndef FIDO_BLE_SERVICE_DEFINE_H
#define FIDO_BLE_SERVICE_DEFINE_H

#include <ble_gatts.h>

#ifdef __cplusplus
extern "C" {
#endif

// 
// BLEサービス共有情報／データ受信用ハンドラー
// 
typedef struct ble_u2f_s ble_u2f_t;
typedef void (*ble_u2f_data_handler_t)(ble_u2f_t *p_u2f, uint8_t *p_data, uint16_t length);

struct ble_u2f_s
{
    uint8_t                  uuid_type;
    uint16_t                 service_handle;

    ble_gatts_char_handles_t u2f_status_handles;
    ble_gatts_char_handles_t u2f_control_point_handles;
    ble_gatts_char_handles_t u2f_control_point_length_handles;
    ble_gatts_char_handles_t u2f_service_revision_bitfield_handles;
    ble_gatts_char_handles_t u2f_service_revision_handles;

    uint16_t                 conn_handle;
    ble_u2f_data_handler_t   data_handler;
};

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BLE_SERVICE_DEFINE_H */
