/* 
 * File:   fido_log.h
 * Author: makmorit
 *
 * Created on 2019/06/17, 17:51
 */
#ifndef FIDO_LOG_H
#define FIDO_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

void fido_log_info(const char *fmt, ...);
void fido_log_error(const char *fmt, ...);
void fido_log_debug(const char *fmt, ...);
void fido_log_print_hexdump_debug(uint8_t *buff, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_LOG_H */

