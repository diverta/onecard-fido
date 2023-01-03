/* 
 * File:   ccid_openpgp_data.h
 * Author: makmorit
 *
 * Created on 2021/02/16, 10:37
 */
#ifndef CCID_OPENPGP_DATA_H
#define CCID_OPENPGP_DATA_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
uint16_t    ccid_openpgp_data_terminate(void *p_capdu, void *p_rapdu);
uint16_t    ccid_openpgp_data_activate(void *p_capdu, void *p_rapdu);
uint16_t    ccid_openpgp_data_put(void *p_capdu, void *p_rapdu);
uint16_t    ccid_openpgp_data_register_key(void *p_capdu, void *p_rapdu, uint16_t key_tag, uint8_t key_status);
void        ccid_openpgp_data_retry(void);
void        ccid_openpgp_data_resume(bool success);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OPENPGP_DATA_H */
