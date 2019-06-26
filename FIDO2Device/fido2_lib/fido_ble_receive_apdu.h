/* 
 * File:   fido_ble_receive_apdu.h
 * Author: makmorit
 *
 * Created on 2019/06/26, 11:32
 */
#ifndef FIDO_BLE_RECEIVE_APDU_H__
#define FIDO_BLE_RECEIVE_APDU_H__

#include "fido_common.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t fido_ble_receive_apdu_header(FIDO_APDU_T *p_apdu, uint8_t *control_point_buffer, uint16_t control_point_buffer_length, uint8_t offset);
bool    fido_ble_receive_apdu_allocate(FIDO_APDU_T *p_apdu);
void    fido_ble_receive_apdu_from_leading(FIDO_APDU_T *p_apdu, uint8_t *control_point_buffer, uint16_t control_point_buffer_length, uint8_t offset);
void    fido_ble_receive_apdu_from_following(FIDO_APDU_T *p_apdu, uint8_t *control_point_buffer, uint16_t control_point_buffer_length);

#ifdef __cplusplus
}
#endif

#endif // FIDO_BLE_RECEIVE_APDU_H__

/** @} */
