#ifndef FIDO_BLE_PAIRING_H__
#define FIDO_BLE_PAIRING_H__

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


void    fido_ble_pairing_delete_bonds(void);
bool    fido_ble_pairing_delete_bonds_response(pm_evt_t const *p_evt);
uint8_t fido_ble_pairing_advertising_flag(void);
bool    fido_ble_pairing_reject_request(ble_evt_t *p_ble_evt);
bool    fido_ble_pairing_allow_repairing(pm_evt_t const *p_evt);
void    fido_ble_pairing_change_mode(void);
void    fido_ble_pairing_reflect_mode_change(fds_evt_t const *const p_evt);
void    fido_ble_pairing_get_mode(ble_u2f_t *p_u2f);
void    fido_ble_pairing_notify_unavailable(pm_evt_t const *p_evt);
void    fido_ble_pairing_on_evt_auth_status(ble_u2f_t *p_u2f, ble_evt_t * p_ble_evt);
void    fido_ble_pairing_on_disconnect(void);
bool    fido_ble_pairing_mode_get(void);


#ifdef __cplusplus
}
#endif

#endif // FIDO_BLE_PAIRING_H__

/** @} */
