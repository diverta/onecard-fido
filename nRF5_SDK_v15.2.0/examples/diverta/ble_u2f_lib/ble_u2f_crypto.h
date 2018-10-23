#ifndef BLE_U2F_CRYPTO_H__
#define BLE_U2F_CRYPTO_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

#ifdef __cplusplus
extern "C" {
#endif


void     ble_u2f_crypto_init(void);
void     ble_u2f_crypto_generate_keypair(void);
uint32_t ble_u2f_crypto_sign(uint8_t *private_key_be, ble_u2f_context_t *p_u2f_context);
bool     ble_u2f_crypto_create_asn1_signature(ble_u2f_context_t *p_u2f_context);

void     ble_u2f_crypto_private_key(uint8_t *p_raw_data, size_t *p_raw_data_size);
void     ble_u2f_crypto_public_key(uint8_t *p_raw_data, size_t *p_raw_data_size);


#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_CRYPTO_H__

/** @} */
