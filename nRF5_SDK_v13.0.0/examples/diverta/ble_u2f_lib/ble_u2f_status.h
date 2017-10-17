#ifndef BLE_U2F_STATUS_H__
#define BLE_U2F_STATUS_H__

#include "sdk_config.h"
#include "ble_stack_handler_types.h"

#include "ble.h"
#include "ble_u2f.h"

#ifdef __cplusplus
extern "C" {
#endif


void     ble_u2f_status_setup(uint8_t command_for_response, uint8_t *data_buffer, uint32_t data_buffer_length);
uint32_t ble_u2f_status_response_send(ble_u2f_t *p_u2f);
void     ble_u2f_status_on_tx_complete(ble_u2f_t *p_u2f);
void     ble_u2f_status_response_ping(ble_u2f_context_t *p_u2f_context);


#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_STATUS_H__

/** @} */
