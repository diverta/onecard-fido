/* 
 * File:   app_log.c
 * Author: makmorit
 *
 * Created on 2023/02/15, 18:31
 */
#include <string.h>
#include <stdint.h>
//
// 共通処理
//
static const char strdup_buffer[1024];

const char *log_strdup(char *string)
{
    strncpy(string, strdup_buffer, sizeof(strdup_buffer));
    return strdup_buffer;
}
