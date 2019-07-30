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
// C --> CPP 呼出用インターフェース
//
void _fido_log_print_hexdump_debug(uint8_t *data, size_t size)
{
    // １行ごとに出力するバイト数
    int c = 8;

    for (int i = 0; i < size; i++) {
        if (i > 0 && i % c == 0) {
            printf("\r\n");
        }
        printf(" %02x", data[i]);
    }
    printf("\r\n");
}
    