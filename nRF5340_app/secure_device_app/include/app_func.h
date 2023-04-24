/* 
 * File:   app_func.h
 * Author: makmorit
 *
 * Created on 2023/02/22, 14:08
 */
#ifndef APP_FUNC_H
#define APP_FUNC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        app_main_data_channel_initialized(void);
bool        app_main_is_data_channel_initialized(void);
void        app_main_hid_configured(void);
void        app_main_hid_data_frame_received(uint8_t *data, size_t size);
void        app_main_hid_request_received(void);
void        app_main_hid_report_sent(void);
void        app_main_ccid_data_frame_received(uint8_t *data, size_t size);
void        app_main_ccid_request_received(void);
void        app_main_ble_data_frame_received(uint8_t *data, size_t size);
void        app_main_ble_request_received(void);
void        app_main_ble_response_sent(void);
void        app_main_app_settings_saved(void);
void        app_main_app_settings_deleted(void);
bool        app_main_button_pressed_short(void);
void        app_main_button_1_pressed(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_FUNC_H */
