/* 
 * File:   ccid_oath_calculate.h
 * Author: makmorit
 *
 * Created on 2022/07/11, 17:03
 */
#ifndef CCID_OATH_CALCULATE_H
#define CCID_OATH_CALCULATE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
uint16_t    ccid_oath_calculate(void *p_capdu, void *p_rapdu);
void        ccid_oath_calculate_digest(uint8_t *secret, uint8_t secret_size, uint8_t *challenge, uint8_t challenge_size, uint8_t *buffer);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OATH_CALCULATE_H */
