/* 
 * File:   ccid_piv_internal_auth.h
 * Author: makmorit
 *
 * Created on 2020/07/23, 16:32
 */
#ifndef CCID_PIV_INTERNAL_AUTH_H
#define CCID_PIV_INTERNAL_AUTH_H

#include "ccid_apdu.h"

#ifdef __cplusplus
extern "C" {
#endif

uint16_t ccid_piv_internal_auth(command_apdu_t *c_apdu, response_apdu_t *r_apdu, uint8_t challenge_pos, uint8_t challenge_size);

#ifdef __cplusplus
}
#endif

#endif /* CCID_PIV_INTERNAL_AUTH_H */
