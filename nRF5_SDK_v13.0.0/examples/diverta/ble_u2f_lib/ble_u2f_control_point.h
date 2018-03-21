#ifndef BLE_U2F_CONTROL_POINT_H__
#define BLE_U2F_CONTROL_POINT_H__

#include "sdk_config.h"
#include "ble_stack_handler_types.h"

#include "ble.h"
#include "ble_srv_common.h"
#include "ble_u2f.h"

#ifdef __cplusplus
extern "C" {
#endif

void ble_u2f_control_point_initialize(void);
void ble_u2f_control_point_receive(ble_gatts_evt_write_t *p_evt_write, ble_u2f_context_t *p_u2f_context);

#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_CONTROL_POINT_H__

/** @} */
