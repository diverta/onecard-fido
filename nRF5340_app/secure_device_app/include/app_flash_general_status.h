/* 
 * File:   app_flash_general_status.h
 * Author: makmorit
 *
 * Created on 2023/04/25, 13:51
 */
#ifndef APP_FLASH_GENERAL_STATUS_H
#define APP_FLASH_GENERAL_STATUS_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool        app_flash_general_status_flag(bool *p_exist);
bool        app_flash_general_status_flag_get(void);
void        app_flash_general_status_flag_set(void);
void        app_flash_general_status_flag_clear(void);
void        app_flash_general_status_flag_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_FLASH_GENERAL_STATUS_H */
