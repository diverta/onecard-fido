#ifndef BLE_U2F_PAIRING_H__
#define BLE_U2F_PAIRING_H__

#include <stdint.h>

#include "ble.h"
#include "ble_u2f.h"

// for pm_evt_t
#include "peer_manager.h"

#ifdef __cplusplus
extern "C" {
#endif


void    ble_u2f_pairing_delete_bonds(ble_u2f_context_t *p_u2f_context);
void    ble_u2f_pairing_delete_bonds_response(ble_u2f_context_t *p_u2f_context, fds_evt_t const *const p_evt);
uint8_t ble_u2f_pairing_advertising_flag(void);
bool    ble_u2f_pairing_reject_request(uint16_t ble_conn_handle, ble_evt_t *p_ble_evt);
bool    ble_u2f_pairing_allow_repairing(pm_evt_t const *p_evt);
void    ble_u2f_pairing_change_mode(ble_u2f_context_t *p_u2f_context);
void    ble_u2f_pairing_reflect_mode_change(ble_u2f_context_t *p_u2f_context, fds_evt_t const *const p_evt);
void    ble_u2f_pairing_get_mode(ble_u2f_t *p_u2f);


#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_PAIRING_H__

/** @} */
