/* 
 * File:   ccid_oath_totp.h
 * Author: makmorit
 *
 * Created on 2022/06/21, 7:55
 */
#ifndef CCID_OATH_TOTP_H
#define CCID_OATH_TOTP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
uint16_t    ccid_oath_totp_set_timestamp(uint8_t *secret, uint8_t *challange);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OATH_TOTP_H */
