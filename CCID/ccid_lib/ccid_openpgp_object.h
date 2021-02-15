/* 
 * File:   ccid_openpgp_object.h
 * Author: makmorit
 *
 * Created on 2021/02/11, 15:46
 */
#ifndef CCID_OPENPGP_OBJECT_H
#define CCID_OPENPGP_OBJECT_H

#include "ccid_apdu.h"
#include "ccid_pin.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// データオブジェクトのタグ
//
#define TAG_OPGP_NONE           0x00
#define TAG_OPGP_PW1            0x01
#define TAG_OPGP_PW3            0x02

//
// 関数群
//
void        ccid_openpgp_object_resume_prepare(command_apdu_t *capdu, response_apdu_t *rapdu);
void        ccid_openpgp_object_resume_process(uint16_t sw);
void        ccid_openpgp_object_pin_clear(void);
bool        ccid_openpgp_object_pin_get(PIN_T *pin, uint8_t **pin_code, uint8_t *pin_size, uint8_t *retries);
bool        ccid_openpgp_object_pin_set(PIN_T *pin, uint8_t *pin_code, uint8_t pin_size, uint8_t retries);
void        ccid_openpgp_object_write_retry(void);
void        ccid_openpgp_object_write_resume(bool success);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OPENPGP_OBJECT_H */
