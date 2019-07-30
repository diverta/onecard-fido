/* 
 * File:   fido_log.cpp
 * Author: makmorit
 *
 * Created on 2019/07/30, 13:24
 */
#include "mbed.h"

//
// fido_log_print_hexdump_debugは、
// マクロで代替えできないので、
// プラットフォームごとに処理の実体を実装
//
void fido_log_print_hexdump_debug(uint8_t *data, size_t size)
{
}
