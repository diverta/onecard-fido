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

//
// BER-TLV関連
//  PIVのリクエスト／レスポンスに
//  格納されるデータオブジェクトは、
//  BER-TLV形式で表現されます
//
typedef struct {
    // Witness
    uint16_t wit_pos;
    int16_t  wit_len;
    // Challenge
    uint16_t chl_pos;
    int16_t  chl_len;
    // Response
    uint16_t rsp_pos;
    int16_t  rsp_len;
    // Exponentiation
    uint16_t exp_pos;
    int16_t  exp_len;
} BER_TLV_INFO;

//
// 関数群
//
uint16_t ccid_piv_general_authenticate(command_apdu_t *capdu, response_apdu_t *rapdu);

#ifdef __cplusplus
}
#endif

#endif /* CCID_PIV_GENERAL_AUTH_H */
