/* 
 * File:   ccid_piv_general_auth.h
 * Author: makmorit
 *
 * Created on 2020/06/03, 14:55
 */
#ifndef CCID_PIV_GENERAL_AUTH_H
#define CCID_PIV_GENERAL_AUTH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
uint16_t ccid_piv_general_authenticate(void *p_capdu, void *p_rapdu);

#ifdef __cplusplus
}
#endif

#endif /* CCID_PIV_GENERAL_AUTH_H */
