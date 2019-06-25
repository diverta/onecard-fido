#ifndef BLE_U2F_CONTROL_POINT_H__
#define BLE_U2F_CONTROL_POINT_H__

#include "ble_gatts.h"

#ifdef __cplusplus
extern "C" {
#endif

void ble_u2f_control_point_initialize(void);
void ble_u2f_control_point_receive(ble_gatts_evt_write_t *p_evt_write);

void    ble_u2f_control_point_receive_frame_count_clear(void);
uint8_t ble_u2f_control_point_receive_frame_count(void);

#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_CONTROL_POINT_H__

/** @} */
