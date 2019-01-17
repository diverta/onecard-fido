#ifndef BLE_U2F_COMMAND_H__
#define BLE_U2F_COMMAND_H__

#include <stdint.h>
#include <stdbool.h>

#include "fds.h"
#include "peer_manager.h"
#include "ble_u2f.h"

#ifdef __cplusplus
extern "C" {
#endif

void ble_u2f_command_initialize_context(void);
void ble_u2f_command_finalize_context(void);
void ble_u2f_command_on_ble_evt_write(ble_u2f_t *p_u2f, ble_gatts_evt_write_t *p_evt_write);
void ble_u2f_command_on_fs_evt(fds_evt_t const *const p_evt);
void ble_u2f_command_keepalive_timer_handler(void *p_context);

bool ble_u2f_command_on_mainsw_event(ble_u2f_t *p_u2f);
bool ble_u2f_command_on_mainsw_long_push_event(ble_u2f_t *p_u2f);

#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_COMMAND_H__

/** @} */
