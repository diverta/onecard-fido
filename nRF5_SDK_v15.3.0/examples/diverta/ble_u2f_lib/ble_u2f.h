#ifndef BLE_U2F_H__
#define BLE_U2F_H__

#include <stdbool.h>

#include "sdk_config.h"

#include "ble.h"
#include "ble_srv_common.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// FIDO Authenticator固有の定義
//
#define NRF_BLE_GATT_MAX_MTU_SIZE   67

// FIDOアライアンス提供の共通ヘッダー
// "u2f.h"より抜粋
#include "fido_common.h"
#include "u2f.h"

// BLEパケット項目のサイズ
#define OPCODE_LENGTH 1
#define HANDLE_LENGTH 2

// ここでBLEの送受信可能最大データ長を調整します
//   U2F Control Point、U2F Status のバッファ長も、
//   この長さに合わせます
#if defined(NRF_BLE_GATT_MAX_MTU_SIZE) && (NRF_BLE_GATT_MAX_MTU_SIZE != 0)
    #define BLE_U2F_MAX_DATA_LEN (NRF_BLE_GATT_MAX_MTU_SIZE - OPCODE_LENGTH - HANDLE_LENGTH)
    #define BLE_GATT_ATT_MTU_PERIPH_SIZE NRF_BLE_GATT_MAX_MTU_SIZE
#else
    #define BLE_U2F_MAX_DATA_LEN (BLE_GATT_MTU_SIZE_DEFAULT - OPCODE_LENGTH - HANDLE_LENGTH)
    #define BLE_GATT_ATT_MTU_PERIPH_SIZE BLE_GATT_MTU_SIZE_DEFAULT
#endif

#define BLE_U2F_MAX_RECV_CHAR_LEN BLE_U2F_MAX_DATA_LEN
#define BLE_U2F_MAX_SEND_CHAR_LEN BLE_U2F_MAX_DATA_LEN


// BLE U2FサービスのUUID
#define BLE_UUID_U2F_SERVICE 0xFFFD


typedef struct ble_u2f_s ble_u2f_t;

typedef void (*ble_u2f_data_handler_t) (ble_u2f_t * p_u2f, uint8_t * p_data, uint16_t length);

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

#endif // BLE_U2F_H__

/** @} */
