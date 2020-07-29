/* 
 * File:   ccid_piv_authenticate.h
 * Author: makmorit
 *
 * Created on 2020/07/23, 16:32
 */
#ifndef CCID_PIV_AUTHENTICATE_H
#define CCID_PIV_AUTHENTICATE_H

#include "ccid_apdu.h"

#ifdef __cplusplus
extern "C" {
#endif

uint16_t ccid_piv_authenticate_internal(command_apdu_t *c_apdu, response_apdu_t *r_apdu, uint8_t challenge_pos, uint8_t challenge_size);
uint16_t ccid_piv_authenticate_ecdh_with_kmk(command_apdu_t *c_apdu, response_apdu_t *r_apdu, uint8_t pubkey_pos, uint8_t pubkey_size);

#ifdef __cplusplus
}
#endif

#endif /* CCID_PIV_AUTHENTICATE_H */
