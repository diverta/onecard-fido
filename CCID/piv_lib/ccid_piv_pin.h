/* 
 * File:   ccid_piv_pin.h
 * Author: makmorit
 *
 * Created on 2020/07/22, 13:05
 */
#ifndef CCID_PIV_PIN_H
#define CCID_PIV_PIN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ccid_apdu.h"

#ifdef __cplusplus
extern "C" {
#endif

bool     ccid_piv_pin_is_validated(void);
void     ccid_piv_pin_set_validated(bool b);
uint8_t *ccid_piv_pin_policy(void);
size_t   ccid_piv_pin_policy_size(void);
bool     ccid_piv_pin_init(void);
uint16_t ccid_piv_pin_set(command_apdu_t *capdu, response_apdu_t *rapdu);
uint16_t ccid_piv_pin_reset(command_apdu_t *capdu, response_apdu_t *rapdu);
uint16_t ccid_piv_pin_auth(command_apdu_t *capdu, response_apdu_t *rapdu);
void     ccid_piv_pin_retry(void);
void     ccid_piv_pin_resume(bool success);

#ifdef __cplusplus
}
#endif

#endif /* CCID_PIV_PIN_H */
