/* 
 * File:   fido_common.c
 * Author: makmorit
 *
 * Created on 2018/12/18, 11:09
 */
#include <stdint.h>

void fido_set_status_word(uint8_t *dest_buffer, uint16_t status_word)
{
    // ステータスワードをビッグエンディアンで格納
    dest_buffer[0] = (status_word >> 8) & 0x00ff;
    dest_buffer[1] = (status_word >> 0) & 0x00ff;
}

void fido_set_uint32_bytes(uint8_t *p_dest_buffer, uint32_t bytes)
{
    // ４バイト整数をビッグエンディアン形式で
    // 指定の領域に格納
    p_dest_buffer[0] = bytes >> 24 & 0xff;
    p_dest_buffer[1] = bytes >> 16 & 0xff;
    p_dest_buffer[2] = bytes >>  8 & 0xff;
    p_dest_buffer[3] = bytes >>  0 & 0xff;
}

void fido_set_uint16_bytes(uint8_t *p_dest_buffer, uint16_t bytes)
{
    // ２バイトの整数をビッグエンディアン形式で
    // 指定の領域に格納
    p_dest_buffer[0] = bytes >>  8 & 0xff;
    p_dest_buffer[1] = bytes >>  0 & 0xff;
}
