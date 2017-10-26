#ifndef BLE_U2F_CRYPTO_ECB_H__
#define BLE_U2F_CRYPTO_ECB_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


void ble_u2f_crypto_ecb_encrypt(uint8_t *packet, uint32_t packet_length, uint8_t *out_packet);
void ble_u2f_crypto_ecb_decrypt(uint8_t *packet, uint32_t packet_length, uint8_t *out_packet);


#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_CRYPTO_ECB_H__

/** @} */
