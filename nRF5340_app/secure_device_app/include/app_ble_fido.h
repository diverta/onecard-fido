/* 
 * File:   app_ble_fido.h
 * Author: makmorit
 *
 * Created on 2021/08/26, 11:50
 */
#ifndef APP_BLE_FIDO_H
#define APP_BLE_FIDO_H

#include <stdbool.h>
#include <stdint.h>
#include <bluetooth/uuid.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// サービス／キャラクタリスティックのUUID
//
// FIDO BLE Service                     FFFD
// FIDO U2F Control Point(RX)           F1D0FFF1-DEAA-ECEE-B42F-C9BA7ED623BB
// FIDO U2F Control Point Length        F1D0FFF3-DEAA-ECEE-B42F-C9BA7ED623BB
// FIDO U2F Status(TX)                  F1D0FFF2-DEAA-ECEE-B42F-C9BA7ED623BB
// FIDO U2F Service Revision Bitfield   F1D0FFF4-DEAA-ECEE-B42F-C9BA7ED623BB
// FIDO U2F Service Revision            2A28
//
#define BT_UUID_FIDO_VAL 0xfffd

#define BT_UUID_FIDO_RX_VAL \
        BT_UUID_128_ENCODE(0xf1d0fff1, 0xdeaa, 0xecee, 0xb42f, 0xc9ba7ed623bb)

#define BT_UUID_FIDO_RX_LEN_VAL \
        BT_UUID_128_ENCODE(0xf1d0fff3, 0xdeaa, 0xecee, 0xb42f, 0xc9ba7ed623bb)

#define BT_UUID_FIDO_TX_VAL \
        BT_UUID_128_ENCODE(0xf1d0fff2, 0xdeaa, 0xecee, 0xb42f, 0xc9ba7ed623bb)

#define BT_UUID_FIDO_SERVICE_REVBF_VAL \
        BT_UUID_128_ENCODE(0xf1d0fff4, 0xdeaa, 0xecee, 0xb42f, 0xc9ba7ed623bb)

#define BT_UUID_FIDO_SERVICE_REV_VAL 0x2a28

#define BT_UUID_FIDO_SERVICE        BT_UUID_DECLARE_16(BT_UUID_FIDO_VAL)
#define BT_UUID_FIDO_RX             BT_UUID_DECLARE_128(BT_UUID_FIDO_RX_VAL)
#define BT_UUID_FIDO_RX_LEN         BT_UUID_DECLARE_128(BT_UUID_FIDO_RX_LEN_VAL)
#define BT_UUID_FIDO_TX             BT_UUID_DECLARE_128(BT_UUID_FIDO_TX_VAL)
#define BT_UUID_FIDO_SERVICE_REVBF  BT_UUID_DECLARE_128(BT_UUID_FIDO_SERVICE_REVBF_VAL)
#define BT_UUID_FIDO_SERVICE_REV    BT_UUID_DECLARE_16(BT_UUID_FIDO_SERVICE_REV_VAL)

//
// 関数群
//
bool        app_ble_fido_send_data(const uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* APP_BLE_FIDO_H */
