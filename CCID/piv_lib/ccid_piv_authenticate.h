/* 
 * File:   ccid_piv_authenticate.h
 * Author: makmorit
 *
 * Created on 2020/07/23, 16:32
 */
#ifndef CCID_PIV_AUTHENTICATE_H
#define CCID_PIV_AUTHENTICATE_H

#include "ccid_apdu.h"
#include "ccid_piv_general_auth.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// 業務処理に関する定義
//
// 暗号化アルゴリズム
#define ALG_TDEA_3KEY       0x03
#define ALG_RSA_2048        0x07
#define ALG_ECC_256         0x11

// 3-key TDES関連
#define TDEA_BLOCK_SIZE     8

// tags for BER-TLV data object
#define TAG_WITNESS         0x80
#define TAG_CHALLENGE       0x81
#define TAG_RESPONSE        0x82
#define TAG_EXPONENT        0x85

#define LENGTH_CHALLENGE    16
#define LENGTH_AUTH_STATE   (5 + LENGTH_CHALLENGE)

// states for authenticate
#define AUTH_STATE_NONE     0
#define AUTH_STATE_EXTERNAL 1
#define AUTH_STATE_MUTUAL   2

//
// 関数群
//
void     ccid_piv_authenticate_reset_context(void);
uint16_t ccid_piv_authenticate_internal(command_apdu_t *c_apdu, response_apdu_t *r_apdu, BER_TLV_INFO *data_obj_info);
uint16_t ccid_piv_authenticate_ecdh_with_kmk(command_apdu_t *c_apdu, response_apdu_t *r_apdu, BER_TLV_INFO *data_obj_info);
uint16_t ccid_piv_authenticate_mutual_request(command_apdu_t *c_apdu, response_apdu_t *r_apdu, BER_TLV_INFO *data_obj_info);
uint16_t ccid_piv_authenticate_mutual_response(command_apdu_t *c_apdu, response_apdu_t *r_apdu, BER_TLV_INFO *data_obj_info);

#ifdef __cplusplus
}
#endif

#endif /* CCID_PIV_AUTHENTICATE_H */
