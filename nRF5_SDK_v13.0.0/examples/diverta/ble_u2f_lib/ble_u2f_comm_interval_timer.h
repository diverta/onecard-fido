#ifndef BLE_U2F_COMM_INTERVAL_TIMER_H__
#define BLE_U2F_COMM_INTERVAL_TIMER_H__

#include <stdint.h>

#include "ble_u2f.h"

#ifdef __cplusplus
extern "C" {
#endif


void ble_u2f_comm_interval_timer_start(ble_u2f_t *p_u2f);
void ble_u2f_comm_interval_timer_stop(ble_u2f_t *p_u2f);


#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_COMM_INTERVAL_TIMER_H__

/** @} */
