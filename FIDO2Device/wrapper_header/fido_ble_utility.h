/* 
 * File:   fido_ble_utility.h
 * Author: makmorit
 *
 * Created on 2022/12/19, 16:54
 */
#ifndef FIDO_BLE_UTILITY_H
#define FIDO_BLE_UTILITY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool        fido_ble_unpairing_request(uint8_t *request_buffer, size_t request_size, uint8_t *response_buffer, size_t *response_size);
void        fido_ble_unpairing_cancel_request(void);
void        fido_ble_unpairing_on_disconnect(void);
void        fido_ble_unpairing_done(bool success, uint16_t peer_id);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BLE_UTILITY_H */
