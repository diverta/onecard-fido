/* 
 * File:   fido_log.c
 * Author: makmorit
 *
 * Created on 2019/06/17, 17:51
 */
#include <stdarg.h>
#include "sdk_common.h"

// for logging informations
//#define NRF_LOG_MODULE_NAME 
#include "nrf_log.h"
//NRF_LOG_MODULE_REGISTER();

void fido_log_error(const char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);
    NRF_LOG_ERROR(fmt);
    va_end(list);
}

void fido_log_debug(const char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);
    NRF_LOG_DEBUG(fmt);
    va_end(list);
}

void fido_log_print_hexdump_debug(uint8_t *buff, size_t size)
{
    int j, k;
    for (j = 0; j < size; j += 64) {
        k = size - j;
        NRF_LOG_HEXDUMP_DEBUG(buff + j, (k < 64) ? k : 64);
    }
}
 