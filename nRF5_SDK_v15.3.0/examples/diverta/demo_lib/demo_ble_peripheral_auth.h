/* 
 * File:   demo_ble_peripheral_auth.h
 * Author: makmorit
 *
 * Created on 2019/10/22, 11:48
 */
#ifndef DEMO_BLE_PERIPHERAL_AUTH_H
#define DEMO_BLE_PERIPHERAL_AUTH_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void demo_ble_peripheral_auth_param_init(void);
void demo_ble_peripheral_auth_param_request(uint8_t *request, size_t request_size);
bool demo_ble_peripheral_auth_param_response(uint8_t cmd_type, uint8_t *response, size_t *response_size);
bool demo_ble_peripheral_auth_scan_enable(void);
bool demo_ble_peripheral_auth_start_scan(void *context);

// for U2F keyhandle, CTAP2 credential ID
size_t demo_ble_peripheral_auth_scan_param_prepare(uint8_t *p_buff);

#ifdef __cplusplus
}
#endif

#endif /* DEMO_BLE_PERIPHERAL_AUTH_H */
