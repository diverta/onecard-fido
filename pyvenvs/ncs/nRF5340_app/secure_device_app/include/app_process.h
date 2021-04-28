/* 
 * File:   app_process.h
 * Author: makmorit
 *
 * Created on 2021/04/28, 10:22
 */
#ifndef APP_PROCESS_H
#define APP_PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        app_process_button_pushed_long(void);
void        app_process_button_pressed_long(void);
void        app_process_button_pressed_short(void);
void        app_process_ble_connected(void);
void        app_process_ble_disconnected(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_PROCESS_H */
