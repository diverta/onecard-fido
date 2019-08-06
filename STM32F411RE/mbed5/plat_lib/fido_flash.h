/* 
 * File:   fido_flash.h
 * Author: makmorit
 *
 * Created on 2019/07/31, 14:28
 */
#ifndef FIDO_FLASH_H__
#define FIDO_FLASH_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

//
//  鍵・証明書の長さを管理
//
#define SKEY_WORD_NUM 8
#define CERT_WORD_NUM 257
#define SKEY_CERT_WORD_NUM (SKEY_WORD_NUM+CERT_WORD_NUM)

void fido_flash_init(void);
void fido_flash_do_process(void);

//
// C --> CPP 呼出用インターフェース
//
bool      _fido_flash_skey_cert_delete(void);
bool      _fido_flash_skey_cert_write(void);
bool      _fido_flash_skey_cert_read(void);
bool      _fido_flash_skey_cert_available(void);
bool      _fido_flash_skey_cert_data_prepare(uint8_t *data, uint16_t length);
uint32_t *_fido_flash_skey_cert_data(void);
uint8_t  *_fido_flash_skey_data(void);
uint8_t  *_fido_flash_cert_data(void);
uint32_t  _fido_flash_cert_data_length(void);

bool      _fido_flash_token_counter_delete(void);

uint8_t  *_fido_flash_password_get(void);
bool      _fido_flash_password_set(uint8_t *random_vector);

#endif // FIDO_FLASH_H__
