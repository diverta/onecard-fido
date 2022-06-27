/* 
 * File:   rtcc.h
 * Author: makmorit
 *
 * Created on 2022/06/01, 12:06
 */
#ifndef RTCC_H
#define RTCC_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        rtcc_init(void);
bool        rtcc_update_timestamp_by_unixtime(uint32_t unixtime);

#ifdef __cplusplus
}
#endif

#endif /* RTCC_H */
