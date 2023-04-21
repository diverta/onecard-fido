/* 
 * File:   app_ble_unpairing.h
 * Author: makmorit
 *
 * Created on 2023/04/20, 10:16
 */
#ifndef APP_BLE_UNPAIRING_H
#define APP_BLE_UNPAIRING_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool        app_ble_unpairing_get_peer_id(uint16_t *peer_id_to_unpair);
bool        app_ble_unpairing_delete_peer_id(uint16_t peer_id_to_unpair);
bool        app_ble_unpairing_delete_all_peers(void (*response_func)(bool));

#ifdef __cplusplus
}
#endif

#endif /* APP_BLE_UNPAIRING_H */
