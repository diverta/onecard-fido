/* 
 * File:   ccid_ykpiv.h
 * Author: makmorit
 *
 * Created on 2020/08/03, 12:16
 */
#ifndef CCID_YKPIV_H
#define CCID_YKPIV_H

#include "ccid_apdu.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// Yubico vendor specific instructions
//
#define YKPIV_INS_SET_MGMKEY            0xff
#define YKPIV_INS_GET_VERSION           0xfd
#define YKPIV_INS_GET_SERIAL            0xf8

//
// 関数群
//
uint16_t ccid_ykpiv_ins_set_mgmkey(command_apdu_t *capdu, response_apdu_t *rapdu);
void     ccid_ykpiv_ins_set_mgmkey_retry(void);
void     ccid_ykpiv_ins_set_mgmkey_resume(bool success);
uint16_t ccid_ykpiv_ins_get_version(command_apdu_t *capdu, response_apdu_t *rapdu);
uint16_t ccid_ykpiv_ins_get_serial(command_apdu_t *capdu, response_apdu_t *rapdu);

#ifdef __cplusplus
}
#endif

#endif /* CCID_YKPIV_H */
