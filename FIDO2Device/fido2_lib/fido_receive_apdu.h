/* 
 * File:   fido_receive_apdu.h
 * Author: makmorit
 *
 * Created on 2018/11/29, 9:57
 */
#ifndef FIDO_RECEIVE_APDU_H
#define FIDO_RECEIVE_APDU_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void    fido_receive_apdu_initialize(void *p_apdu);
uint8_t fido_receive_apdu_header(void *p_apdu, uint8_t *control_point_buffer, uint16_t control_point_buffer_length, uint8_t offset);
void    fido_receive_apdu_from_init_frame(void *p_apdu, uint8_t *control_point_buffer, uint16_t control_point_buffer_length, uint8_t offset);
void    fido_receive_apdu_from_cont_frame(void *p_apdu, uint8_t *control_point_buffer, uint16_t control_point_buffer_length);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_RECEIVE_APDU_H */
