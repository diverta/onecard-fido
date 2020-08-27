/* 
 * File:   atecc_sign.h
 * Author: makmorit
 *
 * Created on 2020/08/24, 9:14
 */
#ifndef ATECC_SIGN_H
#define ATECC_SIGN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool atecc_sign(uint16_t key_id, const uint8_t *msg, uint8_t *signature);
bool atecc_verify_extern(const uint8_t *message, const uint8_t *signature, const uint8_t *public_key);

#ifdef __cplusplus
}
#endif

#endif /* ATECC_SIGN_H */
