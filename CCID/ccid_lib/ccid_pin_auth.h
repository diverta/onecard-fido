/* 
 * File:   ccid_pin_auth.h
 * Author: makmorit
 *
 * Created on 2021/02/11, 11:31
 */
#ifndef CCID_PIN_AUTH_H
#define CCID_PIN_AUTH_H

#include "ccid_pin.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
PIN_T      *ccid_pin_auth_pin_t(PIN_TYPE type);
uint16_t    ccid_pin_auth_verify(PIN_T *pin, uint8_t *buf, uint8_t len);
uint16_t    ccid_pin_auth_get_retries(PIN_T *pin);
uint16_t    ccid_pin_auth_update_retries(PIN_T *pin);

#ifdef __cplusplus
}
#endif

#endif /* CCID_PIN_AUTH_H */
