/* 
 * File:   ccid_piv_pin_update.h
 * Author: makmorit
 *
 * Created on 2020/11/10, 9:23
 */
#ifndef CCID_PIV_PIN_UPDATE_H
#define CCID_PIV_PIN_UPDATE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t ccid_piv_pin_update(uint8_t pin_type, uint8_t *pin_buf);
uint16_t ccid_piv_pin_update_retries(uint8_t pin_type, uint8_t retries);

#ifdef __cplusplus
}
#endif

#endif /* CCID_PIV_PIN_UPDATE_H */
