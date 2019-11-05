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

bool demo_ble_peripheral_auth_param_set(char *p_cdc_buffer, size_t cdc_buffer_size);
void demo_ble_peripheral_auth_param_init(void);
bool demo_ble_peripheral_auth_param_command(uint8_t cmd_type, uint8_t *response, size_t *response_size);
bool demo_ble_peripheral_auth_start_scan(void);

#ifdef __cplusplus
}
#endif

#endif /* DEMO_BLE_PERIPHERAL_AUTH_H */
