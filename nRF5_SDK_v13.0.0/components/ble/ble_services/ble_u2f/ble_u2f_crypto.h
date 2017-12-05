#ifndef BLE_U2F_CRYPTO_H__
#define BLE_U2F_CRYPTO_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

// for nrf_value_length_t
#include "nrf_crypto_types.h"

#ifdef __cplusplus
extern "C" {
#endif


uint32_t * ble_u2f_crypto_compute_publickey(uint32_t *skey_buffer);
uint32_t * ble_u2f_crypto_compute_keyhandle(uint32_t *pkey_buffer);
uint32_t ble_u2f_crypto_sign(uint32_t *skey_buffer, uint32_t *pkey_buffer, uint8_t *signature_base_buffer, uint16_t signature_base_buffer_length);
bool ble_u2f_crypto_create_asn1_signature(nrf_value_length_t *p_signature);


#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_CRYPTO_H__

/** @} */
