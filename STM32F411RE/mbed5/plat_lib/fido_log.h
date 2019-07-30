/* 
 * File:   fido_log.h
 * Author: makmorit
 *
 * Created on 2019/07/30, 12:43
 */
#ifndef FIDO_LOG_H
#define FIDO_LOG_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define fido_log_info(fmt,...)      printf(fmt "\r\n", ##__VA_ARGS__)
#define fido_log_warning(fmt,...)   printf(fmt "\r\n", ##__VA_ARGS__)
#define fido_log_error(fmt,...)     printf(fmt "\r\n", ##__VA_ARGS__)
#define fido_log_debug(fmt,...)     printf(fmt "\r\n", ##__VA_ARGS__)

void fido_log_print_hexdump_debug(uint8_t *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_LOG_H */

