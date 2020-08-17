/* 
 * File:   application_init.h
 * Author: makmorit
 *
 * Created on 2020/08/17, 10:14
 */
#ifndef APPLICATION_INIT_H
#define APPLICATION_INIT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// アプリケーション初期化フラグ
//
typedef enum {
    APP_INI_STAT_NONE,
    APP_INI_STAT_EN_INIT,
    APP_INI_STAT_EN_PROC,
} APP_INI_STAT;

//
// 関数群
//
APP_INI_STAT application_init_status_get(void);
void         application_init_status_set(APP_INI_STAT s);

#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_INIT_H */
