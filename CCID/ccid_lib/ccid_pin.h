/* 
 * File:   ccid_pin.h
 * Author: makmorit
 *
 * Created on 2020/07/22, 10:47
 */
#ifndef CCID_PIN_H
#define CCID_PIN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PIN_DEFAULT_CODE        "123456\xFF\xFF"
#define PIN_DEFAULT_SIZE        8
#define PIN_DEFAULT_RETRY_CNT   3

bool     ccid_piv_pin_auth_failed(void);
uint8_t  ccid_piv_pin_auth_current_retries(void);

uint16_t ccid_piv_pin_auth_verify(uint8_t *buf, uint8_t len);
bool     ccid_piv_pin_auth_get_retries(uint8_t *retries);

#ifdef __cplusplus
}
#endif

#endif /* CCID_PIN_H */
