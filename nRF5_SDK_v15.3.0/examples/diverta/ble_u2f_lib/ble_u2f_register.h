#ifndef BLE_U2F_REGISTER_H__
#define BLE_U2F_REGISTER_H__

#include "fds.h"

#ifdef __cplusplus
extern "C" {
#endif


void ble_u2f_register_do_process(void);
void ble_u2f_register_send_response(fds_evt_t const *const p_evt);


#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_REGISTER_H__

/** @} */
