/* 
 * File:   ccid_ykpiv.h
 * Author: makmorit
 *
 * Created on 2020/08/03, 12:16
 */
#ifndef CCID_YKPIV_H
#define CCID_YKPIV_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// Yubico vendor specific instructions
//
#define YKPIV_INS_SET_MGMKEY            0xff
#define YKPIV_INS_IMPORT_ASYMMETRIC_KEY 0xfe
#define YKPIV_INS_GET_VERSION           0xfd
#define YKPIV_INS_RESET                 0xfb
#define YKPIV_INS_GET_SERIAL            0xf8

//
// 関数群
//
uint16_t ccid_ykpiv_ins_set_mgmkey(void *p_capdu, void *p_rapdu);
void     ccid_ykpiv_ins_set_mgmkey_retry(void);
void     ccid_ykpiv_ins_set_mgmkey_resume(bool success);
uint16_t ccid_ykpiv_ins_import_key(void *p_capdu, void *p_rapdu);
void     ccid_ykpiv_ins_import_key_retry(void);
void     ccid_ykpiv_ins_import_key_resume(bool success);
uint16_t ccid_ykpiv_ins_reset(void *p_capdu, void *p_rapdu);
void     ccid_ykpiv_ins_reset_retry(void);
void     ccid_ykpiv_ins_reset_resume(bool success);
uint16_t ccid_ykpiv_ins_get_version(void *p_capdu, void *p_rapdu);
uint16_t ccid_ykpiv_ins_get_serial(void *p_capdu, void *p_rapdu);

#ifdef __cplusplus
}
#endif

#endif /* CCID_YKPIV_H */
