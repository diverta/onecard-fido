#ifndef BLE_U2F_REGISTER_H__
#define BLE_U2F_REGISTER_H__

#include "ble_u2f.h"
#include "fds.h"

#ifdef __cplusplus
extern "C" {
#endif


void ble_u2f_register_do_process(ble_u2f_context_t *p_u2f_context);
void ble_u2f_register_send_response(ble_u2f_context_t *p_u2f_context, fds_evt_t const *const p_evt);


#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_REGISTER_H__

/** @} */
