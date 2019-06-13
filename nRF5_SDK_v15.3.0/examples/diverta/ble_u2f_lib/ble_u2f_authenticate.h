#ifndef BLE_U2F_AUTHENTICATE_H__
#define BLE_U2F_AUTHENTICATE_H__

#include "ble_u2f.h"
#include "fds.h"

#ifdef __cplusplus
extern "C" {
#endif


void ble_u2f_authenticate_do_process(ble_u2f_context_t *p_u2f_context);
void ble_u2f_authenticate_resume_process(ble_u2f_context_t *p_u2f_context);
void ble_u2f_authenticate_send_response(ble_u2f_context_t *p_u2f_context, fds_evt_t const *const p_evt);


#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_AUTHENTICATE_H__

/** @} */
