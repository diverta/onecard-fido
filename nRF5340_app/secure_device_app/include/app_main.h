/* 
 * File:   app_main.h
 * Author: makmorit
 *
 * Created on 2021/04/02, 15:04
 */
#ifndef APP_MAIN_H
#define APP_MAIN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        app_main_init(void);
bool        app_main_initialized(void);

void        app_main_hid_report_received(uint8_t *data, size_t size);
void        app_main_hid_report_sent(void);
void        app_main_ccid_data_received(uint8_t *data, size_t size);
void        app_main_button_pressed_short(void);
void        app_main_button_1_pressed(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_MAIN_H */
