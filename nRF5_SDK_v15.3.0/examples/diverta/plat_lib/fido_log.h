/* 
 * File:   fido_log.h
 * Author: makmorit
 *
 * Created on 2019/06/17, 17:51
 */
#ifndef FIDO_LOG_H
#define FIDO_LOG_H

#include "nrf_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#define fido_log_info(...)                  NRF_LOG_INFO(__VA_ARGS__)
#define fido_log_warning(...)               NRF_LOG_WARNING(__VA_ARGS__)
#define fido_log_error(...)                 NRF_LOG_ERROR(__VA_ARGS__)
#define fido_log_debug(...)                 NRF_LOG_DEBUG(__VA_ARGS__)
#define fido_log_print_hexdump_debug(...)   NRF_LOG_HEXDUMP_DEBUG(__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* FIDO_LOG_H */

