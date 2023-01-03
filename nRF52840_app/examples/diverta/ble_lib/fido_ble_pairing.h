#ifndef FIDO_BLE_PAIRING_H__
#define FIDO_BLE_PAIRING_H__

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t fido_ble_pairing_advertising_flag(void);
void    fido_ble_pairing_add_service_data_field(void *p_init);
bool    fido_ble_pairing_allow_repairing(void const *pm_evt);
void    fido_ble_pairing_change_mode(void);
void    fido_ble_pairing_get_mode(void);
void    fido_ble_pairing_on_evt_auth_status(void *ble_evt);
void    fido_ble_pairing_on_disconnect(void);
void    fido_ble_pairing_flash_failed(void);
void    fido_ble_pairing_flash_gc_done(void);
void    fido_ble_pairing_flash_updated(void);
void    fido_ble_pairing_reset(void);
void    fido_ble_pairing_flash_deleted(void);
bool    fido_ble_pairing_sleep_after_boot_mode(void);
bool    fido_ble_pairing_get_peer_count(uint8_t *p_count);
bool    fido_ble_pairing_get_peer_id(uint16_t *p_peer_id);
bool    fido_ble_pairing_delete_peer_id(uint16_t peer_id);
bool    fido_ble_pairing_peer_deleted(void *pm_evt);

#ifdef __cplusplus
}
#endif

#endif // FIDO_BLE_PAIRING_H__

/** @} */
