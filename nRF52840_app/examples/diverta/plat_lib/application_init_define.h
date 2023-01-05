/* 
 * File:   application_init_define.h
 * Author: makmorit
 *
 * Created on 2023/01/05, 12:14
 */
#ifndef APPLICATION_INIT_DEFINE_H
#define APPLICATION_INIT_DEFINE_H

#ifdef __cplusplus
extern "C" {
#endif

//
// アプリケーション初期化フラグ
//
typedef enum {
    APP_INI_STAT_NONE,
    APP_INI_STAT_EN_BLEADV,
    APP_INI_STAT_EN_INIT,
    APP_INI_STAT_EN_PROC,
} APP_INI_STAT;

//
// BLEペリフェラル始動判定用タイマー（ミリ秒）
//
#define BLE_ADVERTISE_START_TIMER_MSEC 500

#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_INIT_DEFINE_H */
