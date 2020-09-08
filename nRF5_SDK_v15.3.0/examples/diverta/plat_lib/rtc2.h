/* 
 * File:   rtc2.h
 * Author: makmorit
 *
 * Created on 2020/09/07, 15:49
 */
#ifndef RTC2_H
#define RTC2_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void     rtc2_init(void);
uint32_t rtc2_seconds_get(void);

#ifdef __cplusplus
}
#endif

#endif /* RTC2_H */
