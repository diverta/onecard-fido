/* 
 * File:   application_init.c
 * Author: makmorit
 *
 * Created on 2020/08/17, 10:14
 */
#include "application_init.h"

//
// アプリケーション初期化フラグ
//  0: 初期化処理起動待ち
//  1: 初期化処理が実行可能
//  2: 各業務処理が実行可能
//
static APP_INI_STAT application_init_status = APP_INI_STAT_NONE;

APP_INI_STAT application_init_status_get(void)
{
    return application_init_status;
}

void application_init_status_set(APP_INI_STAT s)
{
    application_init_status = s;
}
