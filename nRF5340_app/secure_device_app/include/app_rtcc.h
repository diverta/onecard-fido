/* 
 * File:   app_rtcc.h
 * Author: makmorit
 *
 * Created on 2022/06/01, 12:09
 */
#ifndef APP_RTCC_H
#define APP_RTCC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool        app_rtcc_initialize(void);
bool        app_rtcc_set_timestamp(uint32_t seconds_since_epoch, uint8_t timezone_diff_hours);
bool        app_rtcc_get_timestamp(char *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* APP_RTCC_H */
