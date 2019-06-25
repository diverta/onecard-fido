#ifndef BLE_U2F_UTIL_H__
#define BLE_U2F_UTIL_H__

#include "ble_u2f.h"

#ifdef __cplusplus
extern "C" {
#endif

bool ble_u2f_signature_data_allocate(ble_u2f_context_t *p_u2f_context);
bool ble_u2f_response_message_allocate(ble_u2f_context_t *p_u2f_context);

#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_UTIL_H__

/** @} */
