/* 
 * File:   ccid_piv_general_auth.h
 * Author: makmorit
 *
 * Created on 2020/06/03, 14:55
 */
#ifndef CCID_PIV_GENERAL_AUTH_H
#define CCID_PIV_GENERAL_AUTH_H

#include "ccid_apdu.h"

#ifdef __cplusplus
extern "C" {
#endif

uint16_t piv_ins_general_authenticate(command_apdu_t *capdu, response_apdu_t *rapdu);
    
#ifdef __cplusplus
}
#endif

#endif /* CCID_PIV_GENERAL_AUTH_H */
