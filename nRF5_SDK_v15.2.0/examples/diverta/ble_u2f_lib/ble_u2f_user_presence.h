#ifndef BLE_U2F_USER_PRESENCE_H__
#define BLE_U2F_USER_PRESENCE_H__

#include <stdint.h>

#include "ble_u2f.h"

#ifdef __cplusplus
extern "C" {
#endif


void ble_u2f_user_presence_init(void);
void ble_u2f_user_presence_terminate(ble_u2f_context_t *p_u2f_context);
void ble_u2f_user_presence_verify_start(ble_u2f_context_t *p_u2f_context);
void ble_u2f_user_presence_verify_end(ble_u2f_context_t *p_u2f_context);


#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_USER_PRESENCE_H__

/** @} */
