#ifndef BLE_U2F_STATUS_RETRY_H__
#define BLE_U2F_STATUS_RETRY_H__

#include <stdint.h>
#include "ble_u2f.h"

#ifdef __cplusplus
extern "C" {
#endif


void ble_u2f_status_retry_error_response(ble_u2f_t *p_u2f, uint16_t err_status_word);


#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_STATUS_RETRY_H__

/** @} */
