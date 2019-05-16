#ifndef BLE_U2F_CONTROL_POINT_H__
#define BLE_U2F_CONTROL_POINT_H__

#include "sdk_config.h"

#include "ble.h"
#include "ble_srv_common.h"
#include "ble_u2f.h"

#ifdef __cplusplus
extern "C" {
#endif

void ble_u2f_control_point_initialize(void);
void ble_u2f_control_point_receive(ble_gatts_evt_write_t *p_evt_write, ble_u2f_context_t *p_u2f_context);

void    ble_u2f_control_point_receive_frame_count_clear(void);
uint8_t ble_u2f_control_point_receive_frame_count(void);

#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_CONTROL_POINT_H__

/** @} */
