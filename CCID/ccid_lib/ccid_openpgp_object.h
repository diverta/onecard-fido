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
void        ccid_openpgp_object_pin_clear(void);
bool        ccid_openpgp_object_pin_get(PIN_T *pin, uint8_t **pin_code, uint8_t *pin_size, uint8_t *retries);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OPENPGP_OBJECT_H */
