/* 
 * File:   ccid_oath_account.h
 * Author: makmorit
 *
 * Created on 2022/06/21, 7:24
 */
#ifndef CCID_ACCOUNT_H
#define CCID_ACCOUNT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
uint16_t    ccid_oath_account_add(void *p_capdu, void *p_rapdu);
uint16_t    ccid_oath_account_delete(void *p_capdu, void *p_rapdu);
uint16_t    ccid_oath_account_reset(void *p_capdu, void *p_rapdu);
void        ccid_oath_account_retry(void);
void        ccid_oath_account_resume(bool success);

#ifdef __cplusplus
}
#endif

#endif /* CCID_ACCOUNT_H */
