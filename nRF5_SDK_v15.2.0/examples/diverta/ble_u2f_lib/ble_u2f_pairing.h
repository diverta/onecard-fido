#ifndef BLE_U2F_PAIRING_H__
#define BLE_U2F_PAIRING_H__

#include <stdint.h>

#include "ble.h"
#include "ble_u2f.h"

// for pm_evt_t
#include "peer_manager.h"

// for fds_evt_t
#include "fds.h"

#ifdef __cplusplus
extern "C" {
#endif


void    ble_u2f_pairing_delete_bonds(ble_u2f_context_t *p_u2f_context);
bool    ble_u2f_pairing_delete_bonds_response(pm_evt_t const *p_evt);
uint8_t ble_u2f_pairing_advertising_flag(void);
bool    ble_u2f_pairing_reject_request(ble_u2f_t *p_u2f, ble_evt_t *p_ble_evt);
bool    ble_u2f_pairing_allow_repairing(pm_evt_t const *p_evt);
void    ble_u2f_pairing_change_mode(ble_u2f_context_t *p_u2f_context);
void    ble_u2f_pairing_reflect_mode_change(ble_u2f_context_t *p_u2f_context, fds_evt_t const *const p_evt);
void    ble_u2f_pairing_get_mode(ble_u2f_t *p_u2f);
void    ble_u2f_pairing_notify_unavailable(ble_u2f_t *p_u2f, pm_evt_t const *p_evt);
void    ble_u2f_pairing_on_evt_auth_status(ble_u2f_t *p_u2f, ble_evt_t * p_ble_evt);
void    ble_u2f_pairing_on_disconnect(void);
bool    ble_u2f_pairing_mode_get(void);


#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_PAIRING_H__

/** @} */
