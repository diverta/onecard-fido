/* 
 * File:   fido_request_apdu.h
 * Author: makmorit
 *
 * Created on 2018/11/29, 9:57
 */
#ifndef FIDO_REQUEST_APDU_H
#define FIDO_REQUEST_APDU_H

#ifdef __cplusplus
extern "C" {
#endif

#include "fido_common.h"

void    fido_request_apdu_initialize(FIDO_APDU_T *p_apdu);
uint8_t fido_request_apdu_header(FIDO_APDU_T *p_apdu, uint8_t *control_point_buffer, uint16_t control_point_buffer_length, uint8_t offset);
void    fido_request_apdu_from_init_frame(FIDO_APDU_T *p_apdu, uint8_t *control_point_buffer, uint16_t control_point_buffer_length, uint8_t offset);
void    fido_request_apdu_from_cont_frame(FIDO_APDU_T *p_apdu, uint8_t *control_point_buffer, uint16_t control_point_buffer_length);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_REQUEST_APDU_H */

