/*
 * File:   ble_peripheral_auth.h
 * Author: makmorit
 *
 * Created on 2019/10/22, 11:48
 */
#ifndef BLE_PERIPHERAL_AUTH_H
#define BLE_PERIPHERAL_AUTH_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void ble_peripheral_auth_param_init(void);
void ble_peripheral_auth_param_request(uint8_t *request, size_t request_size);
bool ble_peripheral_auth_param_response(uint8_t cmd_type, uint8_t *response, size_t *response_size);
bool ble_peripheral_auth_scan_enable(void);
bool ble_peripheral_auth_start_scan(void *context);

// for U2F keyhandle, CTAP2 credential ID
size_t ble_peripheral_auth_scan_param_prepare(uint8_t *p_buff);

#ifdef __cplusplus
}
#endif

#endif /* BLE_PERIPHERAL_AUTH_H */
