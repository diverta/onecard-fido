/* 
 * File:   fido_ble_receive.h
 * Author: makmorit
 *
 * Created on 2019/06/26, 11:32
 */
#ifndef FIDO_BLE_RECEIVE_H
#define FIDO_BLE_RECEIVE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t     fido_ble_receive_header_CMD(void);
uint8_t     fido_ble_receive_header_ERROR(void);
void       *fido_ble_receive_apdu(void);
uint8_t    *fido_ble_receive_apdu_data(void);
uint32_t    fido_ble_receive_apdu_Lc(void);
uint8_t     fido_ble_receive_ctap2_command(void);

void        fido_ble_receive_init(void);
bool        fido_ble_receive_control_point(uint8_t *data, uint16_t length);
void        fido_ble_receive_frame_count_clear(void);
uint8_t     fido_ble_receive_frame_count(void);

void        fido_ble_receive_on_request_received(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_BLE_RECEIVE_H */
