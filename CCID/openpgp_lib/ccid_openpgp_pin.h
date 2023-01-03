/* 
 * File:   ccid_openpgp_pin.h
 * Author: makmorit
 *
 * Created on 2021/02/10, 17:17
 */
#ifndef CCID_OPENPGP_PIN_H
#define CCID_OPENPGP_PIN_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        ccid_openpgp_pin_pw1_mode81_set(bool b);
void        ccid_openpgp_pin_pw1_mode82_set(bool b);
uint8_t     ccid_openpgp_pin_pw1_mode81_get(void);
uint8_t     ccid_openpgp_pin_pw1_mode82_get(void);
void        ccid_openpgp_pin_pw1_mode_clear(void);
void        ccid_openpgp_pin_pw_clear_validated(void);
uint16_t    ccid_openpgp_pin_auth(void *p_capdu, void *p_rapdu);
uint16_t    ccid_openpgp_pin_update(void *p_capdu, void *p_rapdu);
uint16_t    ccid_openpgp_pin_reset(void *p_capdu, void *p_rapdu);
uint16_t    ccid_openpgp_pin_update_reset_code(void *p_capdu, void *p_rapdu);
void        ccid_openpgp_pin_retry(void);
void        ccid_openpgp_pin_resume(bool success);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OPENPGP_PIN_H */
