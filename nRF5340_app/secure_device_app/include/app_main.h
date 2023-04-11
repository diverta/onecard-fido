/* 
 * File:   app_main.h
 * Author: makmorit
 *
 * Created on 2021/04/02, 15:04
 */
#ifndef APP_MAIN_H
#define APP_MAIN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        app_main_init(void);
void        app_main_subsys_init(void);
void        app_main_app_crypto_init_done(void);
void        app_main_app_crypto_do_process(uint8_t event, void (*resume_func)(void));
void        app_main_app_crypto_done(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_MAIN_H */
