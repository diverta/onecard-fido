/* 
 * File:   u2f_request_apdu.h
 * Author: makmorit
 *
 * Created on 2018/11/29, 9:57
 */
#ifndef U2F_REQUEST_APDU_H
#define U2F_REQUEST_APDU_H

#ifdef __cplusplus
extern "C" {
#endif

#include "u2f.h"

void    u2f_control_point_apdu_initialize(U2F_APDU_T *p_apdu);
uint8_t u2f_control_point_apdu_header(U2F_APDU_T *p_apdu, uint8_t *control_point_buffer, uint16_t control_point_buffer_length, uint8_t offset);
void    u2f_control_point_apdu_from_leading(U2F_APDU_T *p_apdu, uint8_t *control_point_buffer, uint16_t control_point_buffer_length, uint8_t offset);
void    u2f_control_point_apdu_from_following(U2F_APDU_T *p_apdu, uint8_t *control_point_buffer, uint16_t control_point_buffer_length);

#ifdef __cplusplus
}
#endif

#endif /* U2F_REQUEST_APDU_H */

