/* 
 * File:   ccid_openpgp_object.h
 * Author: makmorit
 *
 * Created on 2021/02/11, 15:46
 */
#ifndef CCID_OPENPGP_OBJECT_H
#define CCID_OPENPGP_OBJECT_H

#include "ccid_pin.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        ccid_openpgp_object_resume_prepare(void *p_capdu, void *p_rapdu);
void        ccid_openpgp_object_resume_process(uint16_t sw);
void        ccid_openpgp_object_pin_clear(void);
bool        ccid_openpgp_object_pin_get(PIN_T *pin, uint8_t **pin_code, uint8_t *pin_size, uint8_t *retries);
bool        ccid_openpgp_object_pin_set(PIN_T *pin, uint8_t *pin_code, uint8_t pin_size, uint8_t retries);
bool        ccid_openpgp_object_data_get(uint16_t obj_tag, uint8_t **obj_data, size_t *obj_size);
bool        ccid_openpgp_object_data_set(uint16_t obj_tag, uint8_t *obj_data, size_t obj_size);
bool        ccid_openpgp_object_data_delete_all(void);
void        ccid_openpgp_object_write_retry(void);
void        ccid_openpgp_object_write_resume(bool success);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OPENPGP_OBJECT_H */
