/* 
 * File:   atecc_aes.h
 * Author: makmorit
 *
 * Created on 2020/08/24, 14:58
 */
#ifndef ATECC_AES_H
#define ATECC_AES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool atecc_aes_set_persistent_latch(uint16_t write_key_id, uint8_t *write_key);
bool atecc_aes_encrypt(uint8_t *plaintext, size_t plaintext_size, uint8_t *encrypted);
bool atecc_aes_decrypt(uint8_t *encrypted, size_t encrypted_size, uint8_t *decrypted);

#ifdef __cplusplus
}
#endif

#endif /* ATECC_AES_H */
