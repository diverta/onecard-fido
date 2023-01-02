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

#ifdef __cplusplus
extern "C" {
#endif

bool     ccid_piv_pin_is_validated(void);
void     ccid_piv_pin_set_validated(bool b);
uint8_t *ccid_piv_pin_policy(void);
size_t   ccid_piv_pin_policy_size(void);
bool     ccid_piv_pin_init(void);
uint16_t ccid_piv_pin_set(void *p_capdu, void *p_rapdu);
uint16_t ccid_piv_pin_reset(void *p_capdu, void *p_rapdu);
uint16_t ccid_piv_pin_auth(void *p_capdu, void *p_rapdu);
void     ccid_piv_pin_retry(void);
void     ccid_piv_pin_resume(bool success);

#ifdef __cplusplus
}
#endif

#endif /* CCID_PIV_PIN_H */
