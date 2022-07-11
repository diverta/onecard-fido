/* 
 * File:   ccid_oath_calculate.h
 * Author: makmorit
 *
 * Created on 2022/07/11, 17:03
 */
#ifndef CCID_OATH_CALCULATE_H
#define CCID_OATH_CALCULATE_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
uint16_t    ccid_oath_calculate(command_apdu_t *capdu, response_apdu_t *rapdu);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OATH_CALCULATE_H */
