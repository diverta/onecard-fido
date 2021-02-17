/* 
 * File:   ccid_openpgp_data.h
 * Author: makmorit
 *
 * Created on 2021/02/16, 10:37
 */
#ifndef CCID_OPENPGP_DATA_H
#define CCID_OPENPGP_DATA_H

#include "ccid_apdu.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
uint16_t    ccid_openpgp_data_terminate(command_apdu_t *capdu, response_apdu_t *rapdu);
uint16_t    ccid_openpgp_data_activate(command_apdu_t *capdu, response_apdu_t *rapdu);
void        ccid_openpgp_data_retry(void);
void        ccid_openpgp_data_resume(bool success);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OPENPGP_DATA_H */
