#ifndef BLE_U2F_PAIRING_LESC_H__
#define BLE_U2F_PAIRING_LESC_H__

#include <stdint.h>

#include "ble.h"
#include "ble_u2f.h"

#ifdef __cplusplus
extern "C" {
#endif


uint32_t ble_u2f_pairing_lesc_generate_key_pair(void);
bool     ble_u2f_pairing_lesc_on_ble_evt(ble_u2f_t *p_u2f, ble_evt_t * p_ble_evt);

#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_PAIRING_LESC_H__

/** @} */
