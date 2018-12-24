#ifndef BLE_U2F_UTIL_H__
#define BLE_U2F_UTIL_H__

#include <stdint.h>

#include "ble_u2f.h"

#ifdef __cplusplus
extern "C" {
#endif


void dump_octets(uint8_t * data, uint16_t length);

void ble_u2f_send_success_response(ble_u2f_context_t *p_u2f_context);
void ble_u2f_send_error_response(ble_u2f_context_t *p_u2f_context, uint16_t err_status_word);
void ble_u2f_send_command_error_response(ble_u2f_context_t *p_u2f_context, uint8_t err_code);
void ble_u2f_send_keepalive_response(ble_u2f_context_t *p_u2f_context);

bool ble_u2f_signature_data_allocate(ble_u2f_context_t *p_u2f_context);
bool ble_u2f_response_message_allocate(ble_u2f_context_t *p_u2f_context);


#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_UTIL_H__

/** @} */
