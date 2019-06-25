#ifndef BLE_U2F_CONTROL_POINT_APDU_H__
#define BLE_U2F_CONTROL_POINT_APDU_H__

#include "fido_common.h"
#include "ble_u2f.h"

#ifdef __cplusplus
extern "C" {
#endif


// APDUに格納できるデータ長の上限
#define APDU_DATA_MAX_LENGTH 1024

uint8_t ble_u2f_control_point_apdu_header(FIDO_APDU_T *p_apdu, uint8_t *control_point_buffer, uint16_t control_point_buffer_length, uint8_t offset);
bool    ble_u2f_control_point_apdu_allocate(FIDO_APDU_T *p_apdu);
void    ble_u2f_control_point_apdu_from_leading(FIDO_APDU_T *p_apdu, uint8_t *control_point_buffer, uint16_t control_point_buffer_length, uint8_t offset);
void    ble_u2f_control_point_apdu_from_following(BLE_HEADER_T *p_ble_header, FIDO_APDU_T *p_apdu, uint8_t *control_point_buffer, uint16_t control_point_buffer_length);


#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_CONTROL_POINT_APDU_H__

/** @} */
