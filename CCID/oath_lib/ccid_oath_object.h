/* 
 * File:   ccid_oath_object.h
 * Author: makmorit
 *
 * Created on 2022/06/15, 13:36
 */
#ifndef CCID_OATH_OBJECT_H
#define CCID_OATH_OBJECT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
uint16_t    ccid_oath_object_account_set(char *account_name, char *secret, uint8_t property, uint8_t *challange);
uint16_t    ccid_oath_object_account_delete(char *account_name);
void        ccid_oath_object_write_retry(void);
void        ccid_oath_object_write_resume(bool success);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OATH_OBJECT_H */
