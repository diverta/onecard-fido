#ifndef BLE_U2F_AUTHENTICATE_H__
#define BLE_U2F_AUTHENTICATE_H__

#include "fds.h"

#ifdef __cplusplus
extern "C" {
#endif


void ble_u2f_authenticate_do_process(void);
void ble_u2f_authenticate_resume_process(void);
void ble_u2f_authenticate_send_response(fds_evt_t const *const p_evt);


#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_AUTHENTICATE_H__

/** @} */
