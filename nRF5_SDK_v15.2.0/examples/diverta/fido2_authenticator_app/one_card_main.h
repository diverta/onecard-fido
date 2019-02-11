/* 
 * File:   one_card_main.h
 * Author: makmorit
 *
 * Created on 2018/10/08, 10:37
 */

#ifndef ONE_CARD_MAIN_H
#define ONE_CARD_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_ble_gatt.h"
#include "ble_advertising.h"
#include "ble_u2f.h"

void one_card_timers_init(void);
void one_card_buttons_init(void);
void one_card_ble_stack_init(uint8_t conn_cfg_tag, uint32_t p_ram_start);
void one_card_gatt_init(nrf_ble_gatt_t *p_gatt);
void one_card_advertising_init(ble_advertising_init_t *p_init);
void one_card_services_init(void);
void one_card_peer_manager_init(void);
ble_u2f_t *one_card_get_U2F_context(void);

#ifdef __cplusplus
}
#endif

#endif /* ONE_CARD_MAIN_H */

