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
#include <zephyr/kernel.h>

//
// ログ出力関連
//
#include <zephyr/logging/log.h>

#define fido_log_module_register(n)         LOG_MODULE_REGISTER(n)
#define fido_log_info(...)                  LOG_INF(__VA_ARGS__)
#define fido_log_warning(...)               LOG_WRN(__VA_ARGS__)
#define fido_log_error(...)                 LOG_ERR(__VA_ARGS__)
#define fido_log_debug(...)                 LOG_DBG(__VA_ARGS__)
#define fido_log_print_hexdump_debug(...)   LOG_HEXDUMP_DBG(__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* APP_PLATFORM_H */
