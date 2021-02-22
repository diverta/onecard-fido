/* 
 * File:   fido_common.c
 * Author: makmorit
 *
 * Created on 2018/12/18, 11:09
 */
#include <stdint.h>
#include <stddef.h>

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

uint32_t fido_get_uint32_from_bytes(uint8_t *p_src_buffer)
{
    // ４バイトのビッグエンディアン形式配列を、
    // ４バイト整数に変換
    uint32_t uint32;
    uint8_t *p_dest_buffer = (uint8_t *)&uint32;
    p_dest_buffer[0] = p_src_buffer[3];
    p_dest_buffer[1] = p_src_buffer[2];
    p_dest_buffer[2] = p_src_buffer[1];
    p_dest_buffer[3] = p_src_buffer[0];
    return uint32;
}

size_t fido_calculate_aes_block_size(size_t buffer_size)
{    
    // 暗号化対象ブロックサイズを計算
    //   AESの仕様上、16の倍数でなければならない
    size_t block_size;
    size_t block_num = buffer_size / 16;
    size_t block_sum = block_num * 16;
    if (buffer_size == block_sum) {
        block_size = buffer_size;
    } else {
        block_size = (block_num + 1) * 16;
    }

    // 計算されたブロックサイズを戻す
    return block_size;
}

uint8_t *fido_extract_pubkey_in_certificate(uint8_t *cert_data, size_t cert_data_length)
{
    for (size_t i = 3; i < cert_data_length; i++) {
        if (cert_data[i-3] == 0x03 && cert_data[i-2] == 0x42 &&
            cert_data[i-1] == 0x00 && cert_data[i]   == 0x04) {
            // 03 42 00 04 というシーケンスが発見されたら、
            // その先頭アドレスを戻す
            return (cert_data + i + 1);
        }
    }
    return NULL;
}
