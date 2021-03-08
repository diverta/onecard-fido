/* 
 * File:   ccid_openpgp_attr.h
 * Author: makmorit
 *
 * Created on 2021/02/09, 16:27
 */
#ifndef CCID_OPENPGP_ATTR_H
#define CCID_OPENPGP_ATTR_H

#include "ccid_pin.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
uint16_t    ccid_openpgp_attr_get_retries(PIN_TYPE type, uint8_t *retries);
uint16_t    openpgp_attr_get_pw_status(uint8_t *buf, size_t *size);
uint16_t    openpgp_attr_get_login_data(uint8_t *buf, size_t *size);
uint16_t    openpgp_attr_get_url_data(uint8_t *buf, size_t *size);
uint16_t    openpgp_attr_get_digital_sig_counter(uint8_t *buf, size_t *size);
uint16_t    openpgp_attr_set_digital_sig_counter(uint32_t counter);
uint16_t    openpgp_attr_get_name(uint8_t *buf, size_t *size);
uint16_t    openpgp_attr_get_lang(uint8_t *buf, size_t *size);
uint16_t    openpgp_attr_get_sex(uint8_t *buf, size_t *size);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OPENPGP_ATTR_H */
