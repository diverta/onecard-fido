//
//  fido_crypto.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/22.
//
#ifndef fido_crypto_h
#define fido_crypto_h

#include <stdio.h>
#include <stdbool.h>

// 関数群
uint8_t *pin_hash_enc(void);
uint8_t *new_pin_enc(void);
size_t   new_pin_enc_size(void);
uint8_t *pin_auth(void);

uint8_t  generate_pin_hash_enc(const char *cur_pin);
uint8_t  generate_new_pin_enc(const char *new_pin);
uint8_t  generate_pin_auth(bool change_pin);

#endif /* fido_crypto_h */
