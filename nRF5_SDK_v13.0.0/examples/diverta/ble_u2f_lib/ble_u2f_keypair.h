#ifndef BLE_U2F_KEYPAIR_H__
#define BLE_U2F_KEYPAIR_H__

#include <stdint.h>
#include <stdbool.h>

#include "fds.h"
#include "ble_u2f.h"

#ifdef __cplusplus
extern "C" {
#endif


#define SKEY_WORD_NUM 8
#define PKEY_WORD_NUM 16
#define KEYH_WORD_NUM 8
#define KEYPAIR_WORD_NUM (SKEY_WORD_NUM+PKEY_WORD_NUM+KEYH_WORD_NUM)
#define CERT_WORD_NUM 256
#define KEYPAIR_CERT_WORD_NUM (KEYPAIR_WORD_NUM+CERT_WORD_NUM)


void ble_u2f_keypare_install_skey(ble_u2f_context_t *p_u2f_context);
void ble_u2f_keypare_install_cert(ble_u2f_context_t *p_u2f_context);
void ble_u2f_keypare_erase(ble_u2f_context_t *p_u2f_context);

void ble_u2f_keypare_install_skey_response(ble_u2f_context_t *p_u2f_context, fds_evt_t const *const p_evt);
void ble_u2f_keypare_install_cert_response(ble_u2f_context_t *p_u2f_context, fds_evt_t const *const p_evt);
void ble_u2f_keypare_erase_response(ble_u2f_context_t *p_u2f_context, fds_evt_t const *const p_evt);


#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_KEYPAIR_H__

/** @} */
