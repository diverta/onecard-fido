/* 
 * File:   ccid_openpgp_pin.h
 * Author: makmorit
 *
 * Created on 2021/02/10, 17:17
 */
#ifndef CCID_OPENPGP_PIN_H
#define CCID_OPENPGP_PIN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// PIN type
typedef enum {
    OPGP_PIN_PW1,
    OPGP_PIN_PW3,
    OPGP_PIN_RC,
} OPGP_PIN_TYPE;

//
// 関数群
//
OPGP_PIN_TYPE   ccid_openpgp_pin_type_get(void);
void            ccid_openpgp_pin_type_set(OPGP_PIN_TYPE t);
void            ccid_openpgp_pin_pw1_mode81_set(bool b);
void            ccid_openpgp_pin_pw1_mode82_set(bool b);
bool            ccid_openpgp_pin_is_validated(void);
void            ccid_openpgp_pin_set_validated(bool b);
uint16_t        ccid_openpgp_pin_get_retries(uint8_t *retries);
uint16_t        ccid_openpgp_pin_auth_verify(uint8_t *pin_data, size_t pin_size, uint8_t *retries);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OPENPGP_PIN_H */
