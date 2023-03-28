//
//  AES256CBC.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/22.
//
#ifndef AES256CBC_h
#define AES256CBC_h

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "fido_blob.h"

// 関数群
uint8_t aes256_cbc_enc(const fido_blob_t *key, const fido_blob_t *in, fido_blob_t *out);
uint8_t aes256_cbc_dec(const fido_blob_t *key, const fido_blob_t *in, fido_blob_t *out);

#endif /* AES256CBC_h */
