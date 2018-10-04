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


void     ble_u2f_crypto_init(void);
void     ble_u2f_crypto_generate_keypair(void);
uint32_t ble_u2f_crypto_sign(uint8_t *private_key_le, uint8_t *signature_base_buffer, uint16_t signature_base_buffer_length);
bool     ble_u2f_crypto_create_asn1_signature(nrf_value_length_t *p_signature);

nrf_value_length_t *ble_u2f_crypto_private_key(void);
nrf_value_length_t *ble_u2f_crypto_public_key(void);


#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_CRYPTO_H__

/** @} */
