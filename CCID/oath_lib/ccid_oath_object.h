/* 
 * File:   ccid_oath_object.h
 * Author: makmorit
 *
 * Created on 2022/06/15, 13:36
 */
#ifndef CCID_OATH_OBJECT_H
#define CCID_OATH_OBJECT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        ccid_oath_object_resume_prepare(void *p_capdu, void *p_rapdu);
void        ccid_oath_object_resume_response(uint16_t sw);
uint16_t    ccid_oath_object_account_set(char *account_name, uint8_t account_name_size, char *secret, uint8_t secret_size, uint8_t property, uint8_t *challenge);
uint16_t    ccid_oath_object_account_delete(char *account_name, uint8_t account_name_size);
uint16_t    ccid_oath_object_delete_all(void);
uint16_t    ccid_oath_object_account_read(char *account_name, uint8_t account_name_size, char *secret, uint8_t *secret_size, uint8_t *property, uint8_t *challenge, bool *exist);
uint16_t    ccid_oath_object_account_fetch(int (*_fetch_func)(uint8_t *data, size_t size));
void        ccid_oath_object_write_retry(void);
void        ccid_oath_object_write_resume(bool success);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OATH_OBJECT_H */
