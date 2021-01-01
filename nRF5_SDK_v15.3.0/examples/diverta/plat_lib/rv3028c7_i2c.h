/* 
 * File:   rv3028c7_i2c.h
 * Author: makmorit
 *
 * Created on 2020/12/30, 14:38
 */
#ifndef RV3028C7_I2C_H
#define RV3028C7_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool rv3028c7_i2c_init(void);
bool rv3028c7_i2c_set_timestamp(uint32_t seconds_since_epoch, uint8_t timezone_diff_hours);
bool rv3028c7_i2c_get_timestamp_string(char *timestamp_str);
void rv3028c7_i2c_test(void);

#ifdef __cplusplus
}
#endif

#endif /* RV3028C7_I2C_H */
