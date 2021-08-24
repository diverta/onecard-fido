/* 
 * File:   app_platform.h
 * Author: makmorit
 *
 * Created on 2021/08/19, 9:52
 */
#ifndef APP_PLATFORM_H
#define APP_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/types.h>
#include <zephyr.h>

//
// ログ出力関連
//
#include <logging/log.h>

#define fido_log_module_register(n)         LOG_MODULE_REGISTER(n)
#define fido_log_info(...)                  LOG_INF(__VA_ARGS__)
#define fido_log_warning(...)               LOG_WRN(__VA_ARGS__)
#define fido_log_error(...)                 LOG_ERR(__VA_ARGS__)
#define fido_log_debug(...)                 LOG_DBG(__VA_ARGS__)
#define fido_log_print_hexdump_debug(...)   LOG_HEXDUMP_DBG(__VA_ARGS__)

//
// LED操作関数群
//
#include "app_status_indicator.h"

#define fido_status_indicator_none          app_status_indicator_none
#define fido_status_indicator_idle          app_status_indicator_idle
#define fido_status_indicator_busy          app_status_indicator_busy
#define fido_status_indicator_prompt_reset  app_status_indicator_prompt_reset
#define fido_status_indicator_prompt_tup    app_status_indicator_prompt_tup
#define fido_status_indicator_pairing_mode  app_status_indicator_pairing_mode
#define fido_status_indicator_pairing_fail  app_status_indicator_pairing_fail
#define fido_status_indicator_abort         app_status_indicator_abort
#define fido_status_indicator_ble_scanning  app_status_indicator_ble_scanning

//
// 関数群
//
void        usbd_hid_frame_send(uint8_t *buffer_for_send, size_t size);
void        usbd_service_stop_for_bootloader(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_PLATFORM_H */
