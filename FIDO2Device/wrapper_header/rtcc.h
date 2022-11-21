/* 
 * File:   rtcc.h
 * Author: makmorit
 *
 * Created on 2022/11/16, 10:16
 */
#ifndef RTCC_H
#define RTCC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        rtcc_init(void);
bool        rtcc_update_timestamp_by_unixtime(uint32_t unixtime, uint8_t timezone_diff_hours);
bool        rtcc_get_timestamp_string(char *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* RTCC_H */
