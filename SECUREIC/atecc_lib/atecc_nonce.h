/* 
 * File:   atecc_nonce.h
 * Author: makmorit
 *
 * Created on 2020/08/20, 12:52
 */
#ifndef ATECC_NONCE_H
#define ATECC_NONCE_H

#include "atecc_util.h"

#ifdef __cplusplus
extern "C" {
#endif

// RandOut{32} || NumIn{20} || OpCode{1} || Mode{1} || LSB of Param2{1}
#define ATECC_MSG_SIZE_NONCE    (55)

//
// 関数群
//
bool atecc_calculate_nonce(atecc_nonce_in_out_t *param);
bool atecc_nonce_rand(const uint8_t *num_in, uint8_t *rand_out);
bool atecc_nonce_load(uint8_t target, const uint8_t *num_in, uint16_t num_in_size);

#ifdef __cplusplus
}
#endif

#endif /* ATECC_NONCE_H */
