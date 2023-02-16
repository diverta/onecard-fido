/* 
 * File:   fido_ble_define.h
 * Author: makmorit
 *
 * Created on 2023/01/02, 11:36
 */
#ifndef FIDO_BLE_DEFINE_H
#define FIDO_BLE_DEFINE_H

#ifdef __cplusplus
extern "C" {
#endif

//
// FIDO Authenticator固有の定義
//
#define NRF_BLE_GATT_MAX_MTU_SIZE   67

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

//
// BLE U2Fサービスに関する定義
//
#define BLE_UUID_U2F_CONTROL_POINT_CHAR             0xFFF1
#define BLE_UUID_U2F_STATUS_CHAR                    0xFFF2
#define BLE_UUID_U2F_CONTROL_POINT_LENGTH_CHAR      0xFFF3
#define BLE_UUID_U2F_SERVICE_REVISION_BITFIELD_CHAR 0xFFF4
#define BLE_UUID_U2F_SERVICE_REVISION_CHAR          0x2A28

// BLE U2FサービスのUUID
#define BLE_UUID_U2F_SERVICE                        0xFFFD

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BLE_DEFINE_H */
