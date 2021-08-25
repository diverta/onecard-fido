/* 
 * File:   app_board.h
 * Author: makmorit
 *
 * Created on 2021/04/02, 16:25
 */
#ifndef APP_BOARD_H
#define APP_BOARD_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// LED種別
typedef enum _LED_COLOR {
    LED_COLOR_NONE = 0,
    LED_COLOR_RED,
    LED_COLOR_GREEN,
    LED_COLOR_BLUE,
    LED_COLOR_YELLOW
} LED_COLOR;

//
// 関数群
//
uint32_t    app_board_kernel_uptime_ms_get(void);
bool        app_board_get_version_info_csv(uint8_t *info_csv_data, size_t *info_csv_size);
void        app_board_initialize(void);
void        app_board_led_light(LED_COLOR led_color, bool led_on);
void        app_board_prepare_for_deep_sleep(void);
void        app_board_prepare_for_system_reset(void);
void        app_board_prepare_for_bootloader_mode(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_BOARD_H */
