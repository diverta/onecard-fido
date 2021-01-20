/* 
 * File:   fido_ble_service.h
 * Author: makmorit
 *
 * Created on 2019/06/25, 13:30
 */
#ifndef FIDO_BLE_SERVICE_H
#define FIDO_BLE_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

// BLE U2FサービスのUUID
#define BLE_UUID_U2F_SERVICE 0xFFFD

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

//
// 関数群
//
void       fido_ble_advertising_init(void *p_init);
void       fido_ble_services_init(void);
ble_u2f_t *fido_ble_get_U2F_context(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BLE_SERVICE_H */
