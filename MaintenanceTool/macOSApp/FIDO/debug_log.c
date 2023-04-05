//
//  debug_log.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/22.
//
#include "debug_log.h"

// エラーメッセージを保持
static char error_message[1024];

char *log_debug_message(void) {
    return error_message;
}

void log_debug(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsprintf(error_message, fmt, ap);
    va_end(ap);
}
