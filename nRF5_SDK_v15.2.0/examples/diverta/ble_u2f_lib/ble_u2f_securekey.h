#ifndef BLE_U2F_SECUREKEY_H__
#define BLE_U2F_SECUREKEY_H__

#include <stdint.h>
#include <stdbool.h>

#include "fds.h"
#include "ble_u2f.h"

#ifdef __cplusplus
extern "C" {
#endif

void ble_u2f_securekey_install_skey(ble_u2f_context_t *p_u2f_context);
void ble_u2f_securekey_install_cert(ble_u2f_context_t *p_u2f_context);
void ble_u2f_securekey_erase(ble_u2f_context_t *p_u2f_context);

void ble_u2f_securekey_install_skey_response(ble_u2f_context_t *p_u2f_context, fds_evt_t const *const p_evt);
void ble_u2f_securekey_install_cert_response(ble_u2f_context_t *p_u2f_context, fds_evt_t const *const p_evt);
void ble_u2f_securekey_erase_response(ble_u2f_context_t *p_u2f_context, fds_evt_t const *const p_evt);

uint8_t *ble_u2f_securekey_skey(ble_u2f_context_t *p_u2f_context);
uint8_t *ble_u2f_securekey_cert(ble_u2f_context_t *p_u2f_context);
uint32_t ble_u2f_securekey_cert_length(ble_u2f_context_t *p_u2f_context);

#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_SECUREKEY_H__

/** @} */
