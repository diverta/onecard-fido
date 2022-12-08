#ifndef FIDO_BLE_PAIRING_H__
#define FIDO_BLE_PAIRING_H__

#include <stdint.h>

// for pm_evt_t
#include "ble.h"
#include "peer_manager.h"


#ifdef __cplusplus
extern "C" {
#endif

uint8_t fido_ble_pairing_advertising_flag(void);
void    fido_ble_pairing_on_evt_sec_params_request(ble_evt_t const *p_ble_evt);
bool    fido_ble_pairing_allow_repairing(pm_evt_t const *p_evt);
void    fido_ble_pairing_change_mode(void);
void    fido_ble_pairing_get_mode(void);
void    fido_ble_pairing_notify_unavailable(pm_evt_t const *p_evt);
void    fido_ble_pairing_on_evt_auth_status(ble_evt_t * p_ble_evt);
void    fido_ble_pairing_on_disconnect(void);
void    fido_ble_pairing_flash_failed(void);
void    fido_ble_pairing_flash_gc_done(void);
void    fido_ble_pairing_flash_updated(void);
void    fido_ble_pairing_reset(void);
void    fido_ble_pairing_flash_deleted(void);
bool    fido_ble_pairing_sleep_after_boot_mode(void);

#ifdef __cplusplus
}
#endif

#endif // FIDO_BLE_PAIRING_H__

/** @} */
