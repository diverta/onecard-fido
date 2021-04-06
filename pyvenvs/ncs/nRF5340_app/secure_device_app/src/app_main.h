/* 
 * File:   app_main.h
 * Author: makmorit
 *
 * Created on 2021/04/02, 15:04
 */
#ifndef APP_MAIN_H
#define APP_MAIN_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void    app_main_init(void);
bool    app_main_initialized(void);
void    app_bluetooth_start(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_MAIN_H */
