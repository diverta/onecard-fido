/* 
 * File:   atecc_priv.h
 * Author: makmorit
 *
 * Created on 2020/08/19, 14:32
 */
#ifndef ATECC_PRIV_H
#define ATECC_PRIV_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool atecc_priv_write(uint16_t key_id, const uint8_t priv_key[36], uint16_t write_key_id, const uint8_t write_key[32]);

#ifdef __cplusplus
}
#endif

#endif /* ATECC_PRIV_H */
