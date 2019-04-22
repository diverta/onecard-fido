//
//  debug_log.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/22.
//
#ifndef debug_log_h
#define debug_log_h

#include <stdio.h>
#include <stdarg.h>

// 関数群
void  log_debug(const char *fmt, ...);
char *log_debug_message(void);

#endif /* debug_log_h */
