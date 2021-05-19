/* 
 * File:   app_custom.h
 * Author: makmorit
 *
 * Created on 2021/05/06, 9:53
 */
#ifndef APP_CUSTOM_H
#define APP_CUSTOM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        app_custom_hid_report_received(uint8_t *data, size_t size);
void        app_custom_hid_report_sent(void);
void        app_custom_ccid_data_received(uint8_t *data, size_t size);
void        app_custom_button_pressed_short(void);
void        app_custom_button_1_pressed(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_CUSTOM_H */
