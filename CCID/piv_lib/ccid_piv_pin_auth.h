/* 
 * File:   ccid_piv_pin_auth.h
 * Author: makmorit
 *
 * Created on 2020/11/10, 14:48
 */
#ifndef CCID_PIV_PIN_AUTH_H
#define CCID_PIV_PIN_AUTH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PUK_DEFAULT_CODE        "12345678"
#define PIN_DEFAULT_CODE        "123456\xFF\xFF"
#define PIN_DEFAULT_SIZE        8
#define PIN_DEFAULT_RETRY_CNT   3

bool     ccid_piv_pin_auth_failed(uint8_t pin_type);
uint8_t  ccid_piv_pin_auth_current_retries(uint8_t pin_type);

uint16_t ccid_piv_pin_auth_verify(uint8_t pin_type, uint8_t *buf, uint8_t len);
bool     ccid_piv_pin_auth_get_retries(uint8_t pin_type, uint8_t *retries);

#ifdef __cplusplus
}
#endif

#endif /* CCID_PIV_PIN_AUTH_H */
