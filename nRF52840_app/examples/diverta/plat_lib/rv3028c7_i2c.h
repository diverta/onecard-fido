/* 
 * File:   rv3028c7_i2c.h
 * Author: makmorit
 *
 * Created on 2022/11/09, 15:43
 */
#ifndef RV3028C7_I2C_H
#define RV3028C7_I2C_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool rv3028c7_initialize(void);
bool rv3028c7_get_timestamp(char *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* RV3028C7_I2C_H */
