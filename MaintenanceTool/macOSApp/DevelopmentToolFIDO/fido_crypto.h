//
//  fido_crypto.h
//  DevelopmentTool
//
//  Created by Makoto Morita on 2022/06/22.
//
#ifndef fido_crypto_h
#define fido_crypto_h

#include <stdio.h>
#include <stdbool.h>

// 関数群
char   *log_debug_message(void);
uint8_t validate_skey_cert(uint8_t *skey_bytes, size_t skey_bytes_size, uint8_t *cert_bytes, size_t cert_bytes_size);

#endif /* fido_crypto_h */
